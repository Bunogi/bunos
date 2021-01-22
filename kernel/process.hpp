#pragma once

#include <bustd/macros.hpp>
#include <bustd/vector.hpp>
#include <kernel/memory.hpp>
#include <kernel/physicalmalloc.hpp>
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
  BU_NOCOPY(Process)

  Process(Process &&) = default;

  friend class Scheduler;
  Process(void (*fp)());
  Process() = delete;
  bool has_overflowed_stack() const;

  void take_page_table_page(PhysicalAddress &&addr);
  void take_memory_page(PhysicalAddress &&addr);
  void return_from_syscall(u32 return_value);
  bool can_run();
  PhysicalAddress page_dir();
  void update_registers(x86::InterruptFrame *frame);

  // syscall handlers
  void sys_exit(int code);
  // end syscall handlers

private:
  void push_entry_address();
  Registers m_registers;
  VirtualAddress m_kernel_stack_start;
  PhysicalAddress m_page_directory;

  // FIXME: These must be de-allocated when the process exits!
  // Owned physical pages used for page directory entries
  bu::Vector<PhysicalAddress> m_page_table_pages;
  // Owned memory pages used for whatever
  bu::Vector<PhysicalAddress> m_general_memory_pages;

  u32 m_last_run;
  u32 m_kernel_stack_pages;
  void (*m_entry)();
  bool m_can_run{true};
  bool m_has_exit{false};
  bool m_has_run{false};
  bool m_returning_from_syscall{false};
};
} // namespace kernel
