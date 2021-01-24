#include <bustd/assert.hpp>
#include <kernel/elfreader.hpp>
#include <kernel/kprint.hpp>
#include <kernel/process.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/x86/memory.hpp>
#include <kernel/x86/pagemapguard.hpp>
#include <libc/errno.h>
#include <libc/string.h>
#include <libc/sys/syscall.h>

#include <stdio.h>

namespace {
[[noreturn]] void proc_entry(void (*real_entry)()) {
  real_entry();
  // Should only get here if we are a kernel task
  // Might be a better way to do this
  __asm__ volatile("movl %0, %%eax\n"
                   "int $0x80"
                   :
                   : "N"(SYS_EXIT)
                   : "%eax");
  UNREACHABLE();
  while (1) {
  }
}
} // namespace

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

Process::Process(void (*entry)())
    : m_registers(), m_kernel_stack_pages(2), m_entry(entry) {
  m_registers.eip = reinterpret_cast<uintptr_t>(proc_entry);
  m_registers.ebp = 0;
  {
    m_page_directory = allocate_physical_page();
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

  push_entry_address();
}

Process::Process(bu::StringView path) : Process(nullptr) {
  // HACK: undo the push_entry_address call
  // FIXME: Find a better way
  m_registers.esp += 4;

  m_entry = elf::parse(*this, path);
  ASSERT(m_entry);
  push_entry_address();
}

void Process::push_entry_address() {
  const auto index = (m_registers.esp - m_kernel_stack_start.get()) / 4;
  reinterpret_cast<u32 *>(m_kernel_stack_start.ptr())[index] =
      reinterpret_cast<uintptr_t>(m_entry);
  m_registers.esp -= 4;
}

void Process::return_from_syscall(u32 return_value) {
  m_registers.eax = return_value;
  m_returning_from_syscall = true;
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

void Process::update_registers(x86::InterruptFrame *frame) {
  if (m_returning_from_syscall) {
    const auto sys_return = m_registers.eax;
    m_registers.update_from_frame(frame);
    m_registers.eax = sys_return;
    m_returning_from_syscall = false;
  } else {
    m_registers.update_from_frame(frame);
  }
}

void Process::sys_exit(int) {
  // FIXME: Handle exit status
  m_has_exit = true;
}

int Process::sys_write(int fd, const void *buf, size_t bytes) {
  if (fd != 1) {
    return -EBADF;
  }
  // FIXME: This needs some better validation
  if (!buf) {
    return -EINVAL;
  }
  const usize max_bytes_at_once = 256;
  const auto to_write = bu::min(bytes, max_bytes_at_once);
  // printf("Writing %u bytes from syscall\n", to_write);
  print::write(bu::StringView(reinterpret_cast<const char *>(buf), to_write));
  return to_write;
}

} // namespace kernel
