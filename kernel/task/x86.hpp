#pragma once

#include <bustd/vector.hpp>
#include <kernel/memory.hpp>
#include <kernel/x86/interrupts.hpp>

namespace kernel {
class Process;
}

namespace kernel::task::x86 {

class Registers {
public:
  void read_from_frame(const kernel::x86::InterruptFrame *frame);
  void insert_into_frame(kernel::x86::InterruptFrame *frame);

  void reset() {
    m_edi = m_esi = m_ebp = m_dud_esp = m_ebx = m_edx = m_ecx = m_eax =
        m_eflags = m_esp = m_eip = 0;
  }

  void set_entry(void (*entry)()) { m_eip = reinterpret_cast<u32>(entry); }
  void set_stack(u8 *stack) { m_esp = reinterpret_cast<u32>(stack); }
  void set_syscall_target(Process *const proc) {
    m_ebp = reinterpret_cast<u32>(proc);
  }

  auto program_counter() -> VirtualAddress { return VirtualAddress{m_eip}; }

private:
  // pusha
  u32 m_edi, m_esi, m_ebp, m_dud_esp, m_ebx, m_edx, m_ecx, m_eax;

  u32 m_eflags, m_esp, m_eip;
};

class MemoryMapping {
public:
  PhysicalAddress mmu_base;
  bu::Vector<PhysicalAddress> owned_pages;

  void update_kernel_mappings();
  void set_mapping();
};

void register_system_tick_handler();

void enter_scheduler(const VirtualAddress &init_addr);

void setup_syscall_regs(const kernel::x86::InterruptFrame *frame,
                        Registers &regs, VirtualAddress stack_start,
                        usize stack_size, Process *target_process);
} // namespace kernel::task::x86
