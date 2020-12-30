#pragma once

#include <bustd/stddef.hpp>

namespace kernel::interrupt::x86 {
struct InterruptFrame {
  u32 ds;
  // The registers as they were when in the interrupt handler
  u32 edi, esi, ebp, dud_esp, ebx, edx, ecx,
      eax; // dud_esp -> sp at time of interrupt handler
  u32 int_vector, err_code;
  u32 eip, cs, eflags, useresp, ss;
  // esp -> stack pointer where the interrupt happened
};

void initialize();
} // namespace kernel::interrupt::x86
