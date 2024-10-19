#pragma once

#include <bustd/macros.hpp>
#include <bustd/stringview.hpp>
#include <bustd/vector.hpp>
#include <kernel/memory.hpp>
#include <kernel/physicalmalloc.hpp>
#include <kernel/x86/interrupts.hpp>
#include <kernel/x86/memory.hpp>
#include <libc/sys/types.h>

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

enum class ProcessState { Running, InSyscall, StartSyscall, SyscallDone };

class Process {
public:
  BU_NOCOPY(Process)

  Process(Process &&) = default;

  friend class Scheduler;
  Process(bu::StringView name, void (*fp)(), pid_t pid);
  Process(bu::StringView name, bu::StringView file, pid_t pid);
  Process() = delete;
  auto has_overflowed_stack() const -> bool;

  auto pid() const -> pid_t { return m_pid; };

  void take_page_table_page(PhysicalAddress &&addr);
  void take_memory_page(PhysicalAddress &&addr);
  void return_from_syscall(u32 return_value);
  auto can_run() -> bool;
  auto page_dir() -> PhysicalAddress;
  void update_registers(x86::InterruptFrame *frame);

  void start_syscall();

private:
  void (*read_elf(bu::StringView path))();
  void push_entry_address();
  Registers m_registers;
  VirtualAddress m_kernel_stack_start;
  PhysicalAddress m_page_directory;

  // FIXME: These must be de-allocated when the process exits!
  // Owned physical pages used for page directory entries
  bu::Vector<PhysicalAddress> m_page_table_pages;
  // Owned memory pages used for whatever
  bu::Vector<PhysicalAddress> m_general_memory_pages;
  // Kernel stack to use when servicing syscalls
  PhysicalAddress m_kernel_stack_page;

// FIXME: maybe move this somewhere?
#define MAX_SYSCALL_ARG_COUNT 4
  struct SyscallInfo {
    usize syscall;
    usize arguments[MAX_SYSCALL_ARG_COUNT];
    usize retval;
    Registers previous_registers;
    void extract_parameters();
  };
#undef MAX_SYSCALL_ARG_COUNT
  SyscallInfo m_syscall_info;
  ProcessState m_state{ProcessState::Running};

  char m_name[256];

  // syscall handlers
  static void syscall_entry();
  auto do_syscall() -> isize;
  void sys_exit(int code);
  auto sys_open(const char *path, int flags) -> isize;
  auto sys_write(int fd, const void *buf, size_t bytes) -> isize;
  auto sys_read(int fd, void *buf, size_t bytes) -> isize;
  auto sys_close(int fd) -> isize;
  // end syscall handlers

  const pid_t m_pid;
  int m_keyboard_fd{-1}; // FIXME: Replace with better FD handling
  u32 m_last_run;
  u32 m_kernel_stack_pages;
  void (*m_entry)();
  bool m_can_run{true};
  bool m_has_exit{false};
  // TODO: Replace with ProcessState
  bool m_has_run{false};

  // TODO: replace with more generic file descriptor handling
  bool m_has_opened_keyboard{false};
};
} // namespace kernel
