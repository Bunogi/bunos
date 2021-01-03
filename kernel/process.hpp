#pragma once

#include <kernel/x86/interrupts.hpp>
#include <kernel/x86/memory.hpp>

class Scheduler;

namespace kernel {
namespace x86 {
struct Registers {
  // pusha
  u32 edi, esi, ebp, dud_esp, ebx, edx, ecx, eax;

  u32 eflags, esp, eip;
  void update_from_frame(InterruptFrame *frame);
  void prepare_frame(InterruptFrame *frame);
};
} // namespace x86

using x86::Registers;

class Process {
public:
  friend class Scheduler;
  Process(void (*fp)());
  Process() = delete;
  bool has_overflowed_stack() const;
  void push_return_address();

private:
  Registers m_registers;
  void *m_kernel_stack_start;
  uintptr_t m_page_directory;
  u32 m_last_run;
  bool m_has_run;
};
} // namespace kernel
