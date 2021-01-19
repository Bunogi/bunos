#include <bustd/assert.hpp>
#include <kernel/physicalmalloc.hpp>
#include <kernel/process.hpp>
#include <kernel/x86/memory.hpp>
#include <kernel/x86/pagemapguard.hpp>

#include <stdio.h>

namespace kernel {
void x86::Registers::update_from_frame(InterruptFrame *frame) {
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

void x86::Registers::prepare_frame(InterruptFrame *frame) {
  frame->edi = edi;
  frame->esi = esi;
  frame->ebp = ebp;
  frame->ebx = ebx;
  frame->edx = edx;
  frame->ecx = ecx;
  frame->eax = eax;
  frame->eflags = eflags;
}

Process::Process(void (*entry)()) : m_registers(), m_kernel_stack_pages(2) {
  m_registers.eip = reinterpret_cast<uintptr_t>(entry);
  m_registers.ebp = 0;
  {
    m_page_directory = pmem::allocate();
    printf("Pagedir: %p\n", m_page_directory.get());
    const x86::PageMapGuard guard((m_page_directory));
    memcpy(guard.mapped_address(), x86::kernel_page_directory,
           1024 * sizeof(u32));
  }

  m_kernel_stack_start = x86::map_kernel_memory(m_kernel_stack_pages);
  // Stack must be 16 byte aligned
  m_registers.esp =
      reinterpret_cast<VirtualAddress::Type>(m_kernel_stack_start.get()) +
      (PAGE_SIZE * m_kernel_stack_pages - 16);
  m_last_run = 0;
  m_has_run = false;
}

void Process::push_return_address() {
  m_registers.esp -= 4;
  const auto index = (m_registers.esp - m_kernel_stack_start.get()) / 4;
  reinterpret_cast<u32 *>(m_kernel_stack_start.ptr())[index] = m_registers.eip;
}

bool Process::has_overflowed_stack() const {
  return m_registers.esp < m_kernel_stack_start.get();
}

void Process::take_page_table_page(PhysicalAddress &&addr) {
  m_page_table_pages.push(addr);
}

void Process::take_memory_page(PhysicalAddress &&addr) {
  m_general_memory_pages.push(addr);
}

PhysicalAddress Process::page_dir() { return m_page_directory; }

} // namespace kernel
