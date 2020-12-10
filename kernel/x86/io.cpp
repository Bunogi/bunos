#include "io.hpp"

#include <stdio.h>

// X86-specific
namespace kernel::x86::io {
void ensure_ring0_only() {
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

} // namespace kernel::x86::io
