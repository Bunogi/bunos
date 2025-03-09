#include "kernel/task/x86.hpp"
#include "kernel/x86/interrupts.hpp"
#include <bustd/assert.hpp>
#include <bustd/list.hpp>
#include <kernel/filesystem/vfs.hpp>
#include <kernel/panic.hpp>
#include <kernel/process.hpp>
#include <kernel/sched.hpp>
#include <kernel/syscall.hpp>
#include <kernel/task/arch.hpp>
#include <libc/stdio.h>
#include <sys/types.h>

namespace kernel::sched {

Scheduler::Scheduler() : m_current_proc(m_processes.looping_iter()) {
  ASSERT_EQ(s_scheduler, nullptr);
  s_scheduler = this;

  // FIXME: Load more
  Process proc("/bin/yes", m_next_pid++);
  m_processes.emplace_back(SchedInfo{0, proc});

  /*
  // load test processes
  auto &vfs = kernel::Vfs::instance();

  const auto inode = vfs.get_inode_at_path("/bin");
  ASSERT(inode);

  const auto dir = vfs.list_directory(inode->index);
  ASSERT(dir);

  for (const auto &entry : *dir) {
    char buffer[128];
    if (entry.name[0] == '.') {
      continue;
    }
    sprintf(buffer, "/bin/%s", entry.name);
    // FIXME: Need proper emplace() :^)
    Process proc(buffer, m_next_pid++);
    m_processes.emplace_back(SchedInfo{0, proc});
  }
  */

  m_current_proc = m_processes.looping_iter();
}

auto Scheduler::run() -> void {
  ASSERT(!m_processes.empty());
  m_current_proc->proc.set_memory_mapping();

  task::arch::enter_scheduler(m_current_proc->proc.regs().program_counter());
}

auto Scheduler::systick_event(x86::InterruptFrame *frame) -> void {
  ASSERT(s_scheduler);

  s_scheduler->handle_systick(frame);
}

auto Scheduler::handle_systick(x86::InterruptFrame *frame) -> void {
  constexpr auto tick_slices = 1024;
  m_current_proc->run_for_ticks++;
  if (m_current_proc->run_for_ticks >= tick_slices) {
    reschedule(frame);
  }
}

auto Scheduler::reschedule(x86::InterruptFrame *frame) -> void {
  ASSERT(!m_processes.empty());
  m_current_proc->run_for_ticks = 0;

  const pid_t current_pid = m_current_proc->proc.pid();
  pid_t pid = 0;
  usize count = 0;

  auto it = m_current_proc;
  while (count < m_processes.len() && pid != current_pid) {
    it++;
    pid = it->proc.pid();
  }

  if (pid == current_pid) {
    // FIXME: Check if it can actually run. Otherwise we need to run idle
    return;
  }

  m_current_proc->proc.regs().read_from_frame(frame);
  it->proc.set_memory_mapping();
  it->proc.regs().insert_into_frame(frame);
  m_current_proc = it;
}

auto Scheduler::handle_syscall(x86::InterruptFrame *frame) -> void {
  // FIXME: We may be able to do some quick checks here
  auto &proc = m_current_proc->proc;

  proc.regs().read_from_frame(frame);

  proc.start_syscall(frame);
  proc.syscall_regs().insert_into_frame(frame);
}

auto Scheduler::syscall_event(x86::InterruptFrame *frame) -> void {
  ASSERT(s_scheduler);
  s_scheduler->handle_syscall(frame);
}

Scheduler *Scheduler::s_scheduler{nullptr};

void run() {
  Scheduler sched;
  sched.run();
}

} // namespace kernel::sched
