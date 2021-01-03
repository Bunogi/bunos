#include <bustd/assert.hpp>
#include <kernel/x86/io.hpp>
#include <stdio.h>

extern "C" {
// These sucked to write in inline asm
void _x86_out_u8_string(u16 port, const u8 *buffer, usize length);
}

namespace kernel::x86 {
void set_io_permissions() {
  // TODO: Revise when TSS are set up if needed
  // This is ok for now when we don't have  any TSSes setup.
  __asm__ volatile(
      // We don't really care about the other flags as long as IOPL is zero
      "pushf\n"
      "andw $0xCFFF, 2(%esp)\n"
      "popf");
}

void out_u8(u16 port, u8 byte) {
  __asm__ volatile("movw %0, %%dx\n"
                   "movb %1, %%al\n"
                   "outb %%al, %%dx"
                   :
                   : "r"(port), "r"(byte)
                   : "%al", "%dx");
}

void out_u8_string(u16 port, const u8 *buffer, usize length) {
  _x86_out_u8_string(port, buffer, length);
}

u8 in_u8(u16 port) {
  u8 out;
  __asm__ volatile("movw %1, %%dx\n"
                   "in %%dx, %%al\n"
                   "movb %%al, %0"
                   : "=r"(out)
                   : "r"(port)
                   : "%al", "%dx");
  return out;
}

} // namespace kernel::x86
