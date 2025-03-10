#pragma once

#include <bustd/stddef.hpp>

namespace kernel::x86 {
// If this changes, scheduler code has to be updated!
struct InterruptFrame {
  u32 ds;
  // The registers as they were when in the interrupt handler
  u32 edi, esi, ebp, dud_esp, ebx, edx, ecx,
      eax; // dud_esp -> sp at time of interrupt handler
  u32 int_vector, err_code;
  // NOTE: On no priviledge change, useresp and ss will be undefined.
  u32 eip, cs, eflags, useresp, ss;
  // esp -> stack pointer where the interrupt happened
};

// returns ID of that handler
// handler function: Returns whether or not the interrupt was handled
typedef bool (*InterruptHandler)(InterruptFrame *frame);

void initialize_interrupts();

[[gnu::always_inline]] inline void cli() { __asm__ volatile("cli"); }
[[gnu::always_inline]] inline void sti() { __asm__ volatile("sti"); }
[[gnu::always_inline]] inline auto interrupts_enabled() -> bool {
  u32 eflags;
  __asm__ volatile("pushf\n"
                   "popl %0"
                   : "=r"(eflags));
  return (eflags & 0x200) != 0;
}
} // namespace kernel::x86
