#include "serial.hpp"

#include <kernel/x86/io.hpp>

namespace {
namespace Local {
enum class LineStatus {
  DataReady = 0x01,
  OverrunError = 0x02,
  ParityError = 0x04,
  FramingError = 0x08,
  BreakIndicator = 0x10,
  HoldingRegisterEmpty = 0x20,
  TransmitterEmpty = 0x40,
  ImpendingError = 0x80,
};
namespace regs {
constexpr u16 port_offset = 0x3F8;

constexpr u16 transmit_buffer = port_offset + 0;   // if LCR[DLAB] is not set
constexpr u16 baud_rate_div_lsb = port_offset + 0; // only if LCR[DLAB] is set

constexpr u16 interrupt_enable = port_offset + 1;  // if LCR[DLAB] is not set
constexpr u16 baud_rate_div_msb = port_offset + 1; // if LCR[DLAB] is set

constexpr u16 interrupt_detect = port_offset + 2;

constexpr u16 line_control = port_offset + 3;
constexpr u16 modem_control = port_offset + 4;
constexpr u16 line_status = port_offset + 5;
constexpr u16 modem_status = port_offset + 6;
constexpr u16 scratch_pad = port_offset + 7;
} // namespace regs

namespace io = kernel::x86::io;

void init() {
  io::out_u8(regs::line_control, 0x00);     // reset DLAB
  io::out_u8(regs::interrupt_enable, 0x00); // disable interrupts

  // 115200 baud
  io::out_u8(regs::baud_rate_div_lsb, 0x01);
  io::out_u8(regs::baud_rate_div_msb, 0x00);

  // ~DLAB(0x80) | ~Break(0x40) | Parity_en(0x20) | ~even_parity(0x10)
  // parity(0x8) | one stop bit(~0x4) | 8 bit words(3)
  io::out_u8(regs::line_control, 0x2B);
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
  for (usize i = 0; i < length; i++) {
    kernel::x86::io::out_u8(Local::regs::transmit_buffer, buf[i]);
  }
}
void Serial::putchar(const char c) {
  kernel::x86::io::out_u8(Local::regs::transmit_buffer, c);
}
} // namespace kernel::tty::x86
