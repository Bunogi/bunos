#include <bustd/math.hpp>
#include <bustd/ringbuffer.hpp>
#include <bustd/stringview.hpp>
#include <kernel/interruptguard.hpp>
#include <kernel/interrupts.hpp>
#include <kernel/process.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/timer.hpp>
#include <kernel/x86/io.hpp>
#include <kernel/x86/tty/serial.hpp>

#include <stdio.h>

namespace {
using namespace kernel::x86;
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

constexpr u8 com1_interrupt_vector = 0x20 + 0x4;

void init() {
  out_u8(regs::line_control, 0x00);     // reset DLAB
  out_u8(regs::interrupt_enable, 0x00); // disable interrupts

  // set dlab
  out_u8(regs::line_control, 0x80);

  // 115200 baud
  out_u8(regs::baud_rate_div_lsb, 0x01);
  out_u8(regs::baud_rate_div_msb, 0x00);

  // Assumes that we are at least a 16550. bochs does not support 16750 mode so
  // don't enable it.
  out_u8(regs::fifo_control, 0x07);

  // ~DLAB(0x80) | ~Break(0x40) | Parity_en(0x20) | ~even_parity(0x10)
  // parity(0x8) | one stop bit(~0x4) | 8 bit words(0b11)
  out_u8(regs::line_control, 0x2B);

  // Disable everything here
  out_u8(regs::modem_control, 0x00);
}

bool ready_to_send() {
  return (in_u8(regs::line_status) & LineStatus::TransmitterEmpty) != 0;
}

// Must only ever be one
static tty::Serial *s_serial_instance;

bool interrupt_handler(InterruptFrame *) {
  // We have to handle every interrupt sent to us, one at a time
  u8 detect;
  while (((detect = in_u8(regs::interrupt_detect)) & 0x01) == 0) {
    if ((detect & 0x06) != 0x06) {
      const auto line_status = in_u8(regs::line_status);
      if (line_status & LineStatus::TransmitterEmpty) {
        s_serial_instance->transmit();
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
      in_u8(regs::transmit_buffer);
    } else {
      // This should not happen that much so log it and be done
      printf("[Serial] Unknown interrupt detect register value: 0x%.2X\n",
             detect);
    }
  }
  return true;
}
} // namespace

namespace kernel::x86::tty {
Serial::Serial() {
  // TODO: Attempt to detect the first COM port we can actually use
  ASSERT_EQ(s_serial_instance, nullptr);
  init();

  interrupts::register_handler(com1_interrupt_vector, interrupt_handler);

  out_u8(regs::interrupt_enable, 0x02);
  s_serial_instance = this;
}

// Used by the timer system to let us print stuff
Serial *Serial::instance() { return s_serial_instance; }

void Serial::write(const char *buf, const usize length) {
  // If the buffer cannot take any more, we have to wait until it can
  const InterruptGuard guard;
  const auto written =
      m_buffer.write(reinterpret_cast<const u8 *>(buf), length);

  if (written < length) {
    // Block until we can fit the rest of the buffer in
    const auto remaining = length - written;

    if (kernel::in_panic()) {
      while (m_buffer.remaining_space() < remaining) {
        transmit();
      }
    } else {
      x86::sti();
      volatile auto &buffer = m_buffer;
      while (buffer.vol_remaining_space() < remaining) {
        timer::delay(10);
      }
      x86::cli();
    }
    const auto written_again =
        m_buffer.write(reinterpret_cast<const u8 *>(buf) + written, remaining);
    if (written_again + written != length) {
      // TODO: Want to use UNREACHABLE() here, but can't because it will
      // create infinite recursion... do we even need to check this?
      __asm__ volatile("cli\nhlt");
    }
    ASSERT_EQ(written_again + written, length);
  }
} // namespace kernel::x86::tty

void Serial::transmit() {
  const InterruptGuard guard;
  if (ready_to_send()) {
    u8 send_buffer[fifo_size] = {};
    const auto to_send = m_buffer.take(send_buffer, fifo_size);
    out_u8_string(regs::transmit_buffer, send_buffer, to_send);
  }
}

void Serial::flush() {
  if (!m_buffer.is_empty()) {
    if (interrupts::enabled()) {
      while (m_buffer.len() > 0) {
        timer::delay(10);
      }
    } else {
      while (!m_buffer.is_empty()) {
        while (!ready_to_send()) {
        }
        transmit();
      }
    }
  }
}
} // namespace kernel::x86::tty
