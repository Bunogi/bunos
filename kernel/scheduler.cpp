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

void Scheduler::spawn(void (*func)()) {
  ASSERT_NE(s_scheduler, nullptr);
  Process proc(func);
  s_scheduler->m_processes.emplace_back(bu::move(proc));
}

void idle_thread_func() {
  while (1) {
    __asm__ volatile("hlt");
  }
}

void init_process_func() {
  while (1) {
    puts("=== HELLO FROM INIT ===");
    timer::delay(1000 * 5);
  }
}

static volatile bool s_enabled;

Scheduler::Scheduler() {}

void Scheduler::run() {
  ASSERT_NE(s_scheduler, nullptr);

  spawn(idle_thread_func);
  spawn(init_process_func);

  Process test("/bin/test_app");
  s_scheduler->m_processes.emplace_back(bu::move(test));

  // Wait to be woken up to do stuff
  s_enabled = true;
  while (1) {
    __asm__ volatile("sti\nhlt");
  }
}

void Scheduler::wake(x86::InterruptFrame *frame) {
  if (!s_enabled) {
    return;
  }

  auto &current_proc = m_processes.get(m_current_process);
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

Process &Scheduler::current_process() {
  ASSERT_NE(s_scheduler, nullptr);
  return s_scheduler->m_processes.get(s_scheduler->m_current_process);
}

usize Scheduler::get_next_process() {
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

void Scheduler::switch_task(usize new_process, x86::InterruptFrame *frame,
                            bool allow_update) {
  if (!m_first_switch && allow_update) {
    auto &current_proc = m_processes.get(m_current_process);
    current_proc.update_registers(frame);
  } else {
    m_first_switch = false;
  }

  auto &proc = m_processes.get(new_process);
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
  m_current_process = new_process;
  m_ticks_left = ticks_per_task;
}

} // namespace kernel
