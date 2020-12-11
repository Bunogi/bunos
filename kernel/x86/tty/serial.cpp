#include "serial.hpp"

#include <kernel/x86/io.hpp>

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

void wait_until_ready_to_send() {
  while ((io::in_u8(regs::line_status) &
          static_cast<u8>(LineStatus::TransmitterEmpty)) == 0) {
    __asm__ volatile("nop");
  }
}
} // namespace Local
} // namespace

namespace kernel::tty::x86 {
Serial::Serial() {
  // TODO: Attempt to detect the first COM port we can actually use

  Local::init();
}

// TODO: This should really use interrupts when I get that far :)
void Serial::write(const char *buf, usize length) {
  // We enabled 64-byte FIFO, so we can write 64 bytes straight away without
  // waiting
  const auto full_buffer_instances = length / Local::fifo_size;
  const auto rest = length % Local::fifo_size;
  for (usize i = 0; i < full_buffer_instances; i++) {
    Local::wait_until_ready_to_send();
    kernel::x86::io::out_u8_string(Local::regs::transmit_buffer,
                                   reinterpret_cast<const u8 *>(buf) +
                                       i * Local::fifo_size,
                                   Local::fifo_size);
  }
  Local::wait_until_ready_to_send();
  kernel::x86::io::out_u8_string(Local::regs::transmit_buffer,
                                 reinterpret_cast<const u8 *>(buf) +
                                     full_buffer_instances * Local::fifo_size,
                                 rest);
}
void Serial::putchar(const char c) {
  Local::wait_until_ready_to_send();
  kernel::x86::io::out_u8(Local::regs::transmit_buffer, c);
}
} // namespace kernel::tty::x86
