#include "serial.hpp"

#include <bustd/math.hpp>
#include <bustd/ringbuffer.hpp>
#include <bustd/string_view.hpp>
#include <kernel/x86/interruptmanager.hpp>
#include <kernel/x86/io.hpp>

#include <stdio.h>

namespace {
namespace Local {
enum LineStatus : u8 {
  DataReady = 0x01,
  OverrunError = 0x02,
  ParityError = 0x04,
  FramingError = 0x08,
  BreakIndicator = 0x10,
  HoldingRegisterEmpty = 0x20,
  TransmitterEmpty = 0x40,
  ImpendingError = 0x80,
};

constexpr u8 line_error_mask =
    LineStatus::ImpendingError | LineStatus::ParityError |
    LineStatus::FramingError | LineStatus::OverrunError;
namespace regs {
constexpr u16 port_offset = 0x3F8;

constexpr u16 transmit_buffer = port_offset + 0;   // if LCR[DLAB] is not set
constexpr u16 baud_rate_div_lsb = port_offset + 0; // only if LCR[DLAB] is set

constexpr u16 interrupt_enable = port_offset + 1;  // if LCR[DLAB] is not set
constexpr u16 baud_rate_div_msb = port_offset + 1; // if LCR[DLAB] is set

constexpr u16 interrupt_detect = port_offset + 2;
constexpr u16 fifo_control = port_offset + 2; // only available when writing

constexpr u16 line_control = port_offset + 3;
constexpr u16 modem_control = port_offset + 4;
constexpr u16 line_status = port_offset + 5;
constexpr u16 modem_status = port_offset + 6;
constexpr u16 scratch_pad = port_offset + 7;
} // namespace regs

constexpr u16 fifo_size = 16;
namespace io = kernel::x86::io;

constexpr u8 com1_interrupt_vector = 0x20 + 0x4;

void init() {
  io::out_u8(regs::line_control, 0x00);     // reset DLAB
  io::out_u8(regs::interrupt_enable, 0x00); // disable interrupts

  // set dlab
  io::out_u8(regs::line_control, 0x80);

  // 115200 baud
  io::out_u8(regs::baud_rate_div_lsb, 0x01);
  io::out_u8(regs::baud_rate_div_msb, 0x00);

  // Assumes that we are at least a 16550. bochs does not support 16750 mode so
  // don't enable it.
  io::out_u8(regs::fifo_control, 0x07);

  // ~DLAB(0x80) | ~Break(0x40) | Parity_en(0x20) | ~even_parity(0x10)
  // parity(0x8) | one stop bit(~0x4) | 8 bit words(0b11)
  io::out_u8(regs::line_control, 0x2B);

  // Disable everything here
  io::out_u8(regs::modem_control, 0x00);
}

bool ready_to_send() {
  return (io::in_u8(regs::line_status) & LineStatus::TransmitterEmpty) != 0;
}

void wait_until_ready_to_send() {
  while (!ready_to_send()) {
    __asm__ volatile("nop");
  }
}

// Must only ever be one
static kernel::tty::x86::Serial *serial_instance;

bool interrupt_handler(kernel::interrupt::x86::InterruptFrame *frame) {
  // We have to handle every interrupt sent to us, one at a time
  u8 detect;
  while (((detect = io::in_u8(regs::interrupt_detect)) & 0x01) == 0) {
    if ((detect & 0x06) != 0x06) {
      const auto line_status = io::in_u8(regs::line_status);
      if (line_status & LineStatus::TransmitterEmpty) {
        serial_instance->transmit();
      }
      // Transmitter holding register empty: Not useful when FIFOs are enabled
      if ((line_status & ~(LineStatus::TransmitterEmpty |
                           LineStatus::HoldingRegisterEmpty)) != 0) {
        printf("[Serial] Line status has errors, its value is: 0x%.2X\n",
               line_status);
      }
    } else if ((detect & 0x0F) == 0xC) {
      // FIFO, have to read some data
      // just discard it
      io::in_u8(regs::transmit_buffer);
    } else {
      // This should not happen that much so log it and be done
      printf("[Serial] Unknown interrupt detect register value: 0x%.2X\n",
             detect);
    }
  }
  return true;
}
} // namespace Local
} // namespace

namespace kernel::tty::x86 {
Serial::Serial() {
  // TODO: Attempt to detect the first COM port we can actually use
  ASSERT_EQ(Local::serial_instance, nullptr);
  Local::init();

  kernel::interrupt::x86::InterruptManager::instance()->register_handler(
      Local::com1_interrupt_vector, Local::interrupt_handler);

  kernel::x86::io::out_u8(Local::regs::interrupt_enable, 0x02);
  Local::serial_instance = this;
}

void Serial::write(const char *buf, const usize length) {
  const auto guard = kernel::interrupt::x86::InterruptManager::instance()
                         ->disable_interrupts_guarded();
  m_buffer.write(reinterpret_cast<const u8 *>(buf), length);

  if (m_buffer.len() == length) {
    // We are the first to write to the port in a while, so the transmitter
    // empty interrupt has already fired. This means we have to write something
    // now, such that the interrupt fires again when we're done writing.
    transmit();
  }
}
void Serial::putchar(const char) { KERNEL_PANIC(":("); }

void Serial::transmit() {
  const auto guard = kernel::interrupt::x86::InterruptManager::instance()
                         ->disable_interrupts_guarded();
  u8 send_buffer[Local::fifo_size] = {};
  const auto to_send = m_buffer.take(send_buffer, Local::fifo_size);
  kernel::x86::io::out_u8_string(Local::regs::transmit_buffer, send_buffer,
                                 to_send);
}
} // namespace kernel::tty::x86
