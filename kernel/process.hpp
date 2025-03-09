#pragma once

#include <bustd/stringview.hpp>
#include <kernel/memory.hpp>
#include <kernel/task/arch.hpp>
#include <kernel/x86/interrupts.hpp>
#include <sys/types.h>

namespace kernel {
class Process {
public:
  // FIXME: Should not be copyable
  Process(bu::StringView executable, pid_t pid);

  auto set_memory_mapping() -> void { m_memory_mapping.set_mapping(); }
  // FIXME: I bet there's a super clean way to handle these mappings
  auto mmu_base_addr() -> PhysicalAddress { return m_memory_mapping.mmu_base; }

  // FIXME: Does this need to be special? SHould it not be?
  auto take_memory_page(const PhysicalAddress &page) -> void {
    m_memory_mapping.owned_pages.push(page);
  }

  enum State { Running, SyscallTrigger, InSyscall };

  [[nodiscard]] auto state() const -> State { return m_state; }

  auto start_syscall(x86::InterruptFrame *frame) -> void {
    ASSERT_EQ(m_state, Running);
    m_state = State::SyscallTrigger;
    setup_syscall_regs(frame, m_syscall_regs, m_kernel_stack,
                       m_kernel_stack_size, this);
  }

  [[nodiscard]] auto pid() const -> pid_t { return m_pid; }
  [[nodiscard]] auto regs() const -> task::arch::Registers { return m_regs; }
  [[nodiscard]] auto syscall_regs() const -> task::arch::Registers {
    return m_syscall_regs;
  }

private:
  State m_state{State::Running};
  // FIXME: Cleanup on exit
  task::arch::Registers m_regs;
  task::arch::MemoryMapping m_memory_mapping;

  pid_t m_pid;

  // Syscall thread
  //  FIXME: Cleanup on exit
  task::arch::Registers m_syscall_regs;
  // FIXME: Doesn't need to be always mapped
  VirtualAddress m_kernel_stack;
  usize m_kernel_stack_size;
};
} // namespace kernel
