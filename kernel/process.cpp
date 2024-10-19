#include <bustd/assert.hpp>
#include <bustd/stringview.hpp>
#include <kernel/elfreader.hpp>
#include <kernel/kprint.hpp>
#include <kernel/process.hpp>
#include <kernel/process/keyboardfile.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/x86/memory.hpp>
#include <kernel/x86/pagemapguard.hpp>
#include <libc/errno.h>
#include <libc/fcntl.h>
#include <libc/stdio.h>
#include <libc/string.h>
#include <libc/sys/syscall.h>

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

Process::Process(const bu::StringView name, void (*const entry)(),
                 const pid_t pid)
    : m_registers(), m_pid(pid), m_kernel_stack_pages(2), m_entry(entry) {

  strncpy(m_name, name.data(), name.len());

  m_registers.eip = reinterpret_cast<uintptr_t>(proc_entry);
  m_registers.ebp = 0;
  {
    m_page_directory = allocate_physical_page();
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

Process::Process(const bu::StringView name, const bu::StringView path,
                 const pid_t pid)
    : Process(name, nullptr, pid) {
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

auto Process::has_overflowed_stack() const -> bool {
  return m_registers.esp < m_kernel_stack_start.get();
}

void Process::take_page_table_page(PhysicalAddress &&addr) {
  m_page_table_pages.push(addr);
}

void Process::take_memory_page(PhysicalAddress &&addr) {
  m_general_memory_pages.push(addr);
}

auto Process::page_dir() -> PhysicalAddress { return m_page_directory; }

void Process::update_registers(x86::InterruptFrame *const frame) {
  m_registers.update_from_frame(frame);
  switch (m_state) {
  case ProcessState::StartSyscall:
    m_syscall_info.previous_registers.update_from_frame(frame);
    m_registers.eip = reinterpret_cast<u32>(&Process::syscall_entry);
    m_syscall_info.extract_parameters();
    break;
  case ProcessState::SyscallDone:
    memcpy(&m_registers, &m_syscall_info.previous_registers, sizeof(Registers));
    m_registers.eax = m_syscall_info.retval;
    m_state = ProcessState::Running;
    // TODO: with a separate kernel stack, we have to update esp here
    break;
  case ProcessState::InSyscall:
  case ProcessState::Running:
    break;
  }
}

void Process::SyscallInfo::extract_parameters() {
  syscall = previous_registers.eax;
  arguments[0] = previous_registers.ebx;
  arguments[1] = previous_registers.ecx;
  arguments[2] = previous_registers.edx;
}

void Process::start_syscall() {
  switch (m_state) {
  case ProcessState::Running:
    m_state = ProcessState::StartSyscall;
    return;
  case ProcessState::InSyscall:
    // Yielding from inside a syscall, no-op
    return;
  case ProcessState::StartSyscall:
    UNREACHABLE();
  case ProcessState::SyscallDone:
    // Syscall done, don't have to do anything
    return;
  default:
    UNREACHABLE();
  }
}

// Entry point for syscalls. We must not return from this function accidentally.
void Process::syscall_entry() {
  auto &proc = Scheduler::current_process();
  proc.m_state = ProcessState::InSyscall;
  proc.m_syscall_info.retval = proc.do_syscall();
  proc.m_state = ProcessState::SyscallDone;
  // Returning from here would be bad. So instead, trigger another syscall,
  // which reschedules us.
  asm volatile("int $0x80");
}

auto Process::do_syscall() -> isize {
  switch (m_syscall_info.syscall) {
  case SYS_READ:
    return sys_read(m_syscall_info.arguments[0],
                    reinterpret_cast<void *>(m_syscall_info.arguments[1]),
                    m_syscall_info.arguments[2]);
  case SYS_EXIT:
    sys_exit(m_syscall_info.arguments[0]);
    return 0;
  case SYS_WRITE:
    return sys_write(
        m_syscall_info.arguments[0],
        reinterpret_cast<const void *>(m_syscall_info.arguments[1]),
        m_syscall_info.arguments[2]);
  case SYS_OPEN:
    return sys_open(reinterpret_cast<const char *>(m_syscall_info.arguments[0]),
                    m_syscall_info.arguments[1]);
  case SYS_CLOSE:
    return sys_close(m_syscall_info.arguments[0]);
  default:
    printf("[syscall] Unknown syscall %u\n", m_syscall_info.syscall);
    // Invalid syscall :(
    return -ENOSYS;
  }
}

void Process::sys_exit(int code) {
  // FIXME: Handle exit status
  m_has_exit = true;

  // FIXME: destroy all FDs
  if (m_keyboard_fd != -1) {
    sys_close(m_keyboard_fd);
  }

  printf("[kernel] Process %s exited with %d\n", m_name, code);

  Scheduler::run_next_test_proc(m_pid);
}

auto Process::sys_close(int fd) -> isize {
  // FIXME: better fd stuff
  if (fd < 1) {
    return -EBADF;
  }
  if (m_keyboard_fd != -1 && fd == m_keyboard_fd) {
    auto &keyboard = process::KeyboardFile::instance();
    ASSERT(keyboard.close(m_pid));
    m_keyboard_fd = -1;
    return 0;
  }
  return -EBADF;
}

// FIXME: Validation
auto Process::sys_open(const char *const file_path, int flags) -> isize {
  if (strcmp(file_path, "/dev/keyboard") != 0) {
    return -EINVAL;
  }
  if (flags != O_RDONLY) {
    return -EINVAL;
  }

  auto &keyboard = process::KeyboardFile::instance();
  if (!keyboard.open(m_pid)) {
    return -EACCES;
  }
  // FIXME: assign file descriptors
  m_keyboard_fd = 30;
  return m_keyboard_fd;
}

auto Process::sys_write(int fd, const void *buf, size_t bytes) -> isize {
  if (fd != 1) {
    return -EBADF;
  }
  // FIXME: This needs some better validation
  if (!buf) {
    return -EINVAL;
  }
  const usize max_bytes_at_once = 256;
  const auto to_write = bu::min(bytes, max_bytes_at_once);
  print::write(bu::StringView(reinterpret_cast<const char *>(buf), to_write));
  return to_write;
}

auto Process::sys_read(const int fd, void *const buf,
                       const size_t bytes) -> isize {
  if (m_keyboard_fd != -1) {
    if (m_keyboard_fd != fd) {
      return -EBADF;
    }
  } else {
    return -EBADF;
  }
  if (bytes == 0) {
    return 0;
  }
  if (!buf) {
    return -EINVAL;
  }

  size_t numread = 0;
  auto &keyboard = process::KeyboardFile::instance();
  while (numread < bytes) {
    // TODO: When reading a file, we need to stop at EOF.
    const auto read =
        keyboard.read(reinterpret_cast<u8 *>(buf), bytes - numread);
    if (read == 0) {
      // yield for now
      asm volatile("int $0x80");
      continue;
    }
    // TODO: have to handle this too
    ASSERT(read > 0);
    numread += read;
  }

  return numread;
}

} // namespace kernel
