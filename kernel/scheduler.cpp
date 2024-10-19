#include <bustd/algorithm.hpp>
#include <kernel/filesystem/inode.hpp>
#include <kernel/filesystem/vfs.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/timer.hpp>
#include <kernel/x86/memory.hpp>
#include <stdio.h>

kernel::Scheduler *s_scheduler;

constexpr usize ticks_per_task = 10;

extern "C" {
extern void _x86_task_switch();

volatile u32 new_esp;
volatile u32 new_eip;
}

namespace kernel {
void Scheduler::init() {
  ASSERT_EQ(s_scheduler, nullptr);
  s_scheduler = new Scheduler();
}

void Scheduler::spawn(const bu::StringView name, void (*func)()) {
  ASSERT_NE(s_scheduler, nullptr);
  Process proc(name, func, s_scheduler->next_pid());
  s_scheduler->m_processes.emplace_back(bu::move(proc));
}

void idle_thread_func() {
  while (1) {
    __asm__ volatile("hlt");
  }
}

static volatile bool s_enabled;

Scheduler::Scheduler() {}

void Scheduler::run() {
  ASSERT_NE(s_scheduler, nullptr);

  spawn("idle", idle_thread_func);

  Process test("test_app", "/bin/test_app", s_scheduler->next_pid());
  s_scheduler->m_processes.emplace_back(bu::move(test));
  s_scheduler->spawn_test_processes();

  // Wait to be woken up to do stuff
  s_enabled = true;
  while (1) {
    __asm__ volatile("sti\nhlt");
  }
}

auto Scheduler::next_pid() -> pid_t { return m_next_pid++; }

void Scheduler::wake(x86::InterruptFrame *frame) {
  if (!s_enabled) {
    return;
  }

  auto &current_proc = current_process();
  if (current_proc.m_has_exit) {
    m_processes.remove(m_current_process);
    auto next_proc = get_next_process();
    switch_task(next_proc, frame, false);
  } else if (m_ticks_left == 0) {
    auto next_proc = get_next_process();
    switch_task(next_proc, frame, true);
  } else {
    m_ticks_left--;
  }

  m_this_tick++;
}

void Scheduler::yield(x86::InterruptFrame *frame) {
  ASSERT_NE(s_scheduler, nullptr);
  s_scheduler->m_ticks_left = 0;
  s_scheduler->wake(frame);
}

auto Scheduler::current_process() -> Process & {
  ASSERT_NE(s_scheduler, nullptr);
  return s_scheduler->find_process(s_scheduler->m_current_process);
}

auto Scheduler::get_next_process() -> usize {
  usize lowest_run = 0;
  usize max_delta = 0;
  usize index = 0;
  for (auto &i : m_processes) {
    usize delta;
    // Check overflow
    if (i.m_last_run > m_this_tick) {
      delta = i.m_last_run - m_this_tick;
    } else {
      delta = m_this_tick - i.m_last_run;
    }

    if (delta > max_delta) {
      max_delta = delta;
      lowest_run = index;
    }
    index++;
  }

  return lowest_run;
}

void Scheduler::switch_task(pid_t new_pid, x86::InterruptFrame *frame,
                            bool allow_update) {
  if (!m_first_switch && allow_update) {
    current_process().update_registers(frame);
  } else {
    m_first_switch = false;
  }

  auto &proc = find_process(new_pid);
#ifdef __IS_X86__
  _x86_set_page_directory(proc.page_dir().get());
#else
#error Expected x86!
#endif

  if (!proc.m_has_run) {
    proc.push_entry_address();
    proc.m_has_run = true;
  }
  new_esp = proc.m_registers.esp;
  new_eip = proc.m_registers.eip;

  proc.m_registers.prepare_frame(frame);
  frame->eip = reinterpret_cast<uintptr_t>(&_x86_task_switch);
  frame->eflags |= 0x200;
  proc.m_last_run = m_this_tick;
  m_current_process = new_pid;
  m_ticks_left = ticks_per_task;
}

void Scheduler::spawn_test_processes() {
  auto &vfs = Vfs::instance();
  const auto inode = vfs.get_inode_at_path("/bin/test");
  ASSERT(inode);
  ASSERT_EQ(inode->type, filesystem::InodeType::Directory);

  const auto dir = vfs.list_directory(inode->index);
  ASSERT(dir);

  for (const auto &entry : *dir) {
    char name_buffer[50];
    if (entry.name[0] == '.') {
      continue;
    }
    sprintf(name_buffer, "/bin/test/%s", entry.name);
    Process test(name_buffer, name_buffer, next_pid());
    m_test_processes.emplace_back(bu::move(test));
  }

  // start her up
  m_processes.take_back(m_test_processes, 0);
  m_current_test_process = m_processes.back().pid();
}

auto Scheduler::find_process(const pid_t pid) -> Process & {
  auto result =
      bu::find_if(m_processes.begin(), m_processes.end(),
                  [&](const auto &proc) { return proc.pid() == pid; });
  ASSERT_NE(result, m_processes.end());
  return *result;
}

void Scheduler::run_next_test_proc(pid_t exited_pid) {
  if (s_scheduler->m_current_test_process != exited_pid) {
    return;
  }
  if (s_scheduler->m_test_processes.len() == 0) {
    return;
  }
  s_scheduler->m_current_test_process =
      s_scheduler->m_test_processes.front().pid();
  s_scheduler->m_processes.take_back(s_scheduler->m_test_processes, 0);
}

} // namespace kernel
