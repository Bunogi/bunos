#include <bustd/assert.hpp>
#include <kernel/process.hpp>
#include <kernel/x86/memory.hpp>

extern "C" {
extern void _x86_process_entry();
}

namespace kernel {
void x86::Registers::update_from_frame(interrupt::x86::InterruptFrame *frame) {
  edi = frame->edi;
  esi = frame->esi;
  ebp = frame->ebp;
  // Privilege level change.
  // We can just calculate what the esp was if there was no change.
  if (frame->cs != 0x8) {
    esp = frame->useresp;
  } else {
    esp = reinterpret_cast<uintptr_t>(&frame->useresp);
  }

  ebx = frame->ebx;
  edx = frame->edx;
  ecx = frame->ecx;
  eax = frame->eax;
  eflags = frame->eflags;
  eip = frame->eip;
}

void x86::Registers::prepare_frame(interrupt::x86::InterruptFrame *frame) {
  frame->edi = edi;
  frame->esi = esi;
  frame->ebp = ebp;
  frame->ebx = ebx;
  frame->edx = edx;
  frame->ecx = ecx;
  frame->eax = eax;
  frame->eflags = eflags;
}

Process::Process(void (*entry)()) : m_registers() {
  m_registers.eip = reinterpret_cast<uintptr_t>(entry);
  m_registers.ebp = 0;
  m_page_directory = kernel::memory::x86::virt_to_phys_addr(
      reinterpret_cast<uintptr_t>(memory::x86::kernel_page_directory));
  m_kernel_stack_start = memory::x86::map_kernel_memory(1);
  // Stack must be 16 byte aligned
  m_registers.esp =
      reinterpret_cast<uintptr_t>(m_kernel_stack_start) + (4096 - 16);
  m_last_run = 0;
  m_has_run = false;
}

void Process::push_return_address() {
  m_registers.esp -= 4;
  const auto index =
      (m_registers.esp - reinterpret_cast<uintptr_t>(m_kernel_stack_start)) / 4;
  reinterpret_cast<u32 *>(m_kernel_stack_start)[index] = m_registers.eip;
}

bool Process::has_overflowed_stack() const {
  return m_registers.esp < reinterpret_cast<uintptr_t>(m_kernel_stack_start);
}
} // namespace kernel
