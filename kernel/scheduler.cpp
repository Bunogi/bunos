#include <kernel/scheduler.hpp>
#include <kernel/timer.hpp>
#include <kernel/x86/memory.hpp>
#include <stdio.h>

kernel::Scheduler *s_scheduler;

constexpr usize ticks_per_task = 10;

extern "C" {
extern void _x86_task_switch();
extern void _test();

volatile u32 new_esp;
volatile u32 new_eip;
}

namespace kernel {
void Scheduler::init() {
  ASSERT_EQ(s_scheduler, nullptr);
  s_scheduler = new Scheduler();
}

void Scheduler::spawn(Process &&p) {
  ASSERT_NE(s_scheduler, nullptr);
  s_scheduler->m_processes.push(bu::forward(p));
}

void idle_thread_func() {
  while (1) {
    __asm__ volatile("hlt");
  }
  UNREACHABLE();
}

void Scheduler::run() {
  ASSERT_NE(s_scheduler, nullptr);

  Process idle_thread(idle_thread_func);

  spawn(bu::move(idle_thread));

  // Wait to be woken up to do stuff
  while (1) {
    __asm__ volatile("sti\nhlt");
  }
}

void Scheduler::wake(kernel::interrupt::x86::InterruptFrame *frame) {
  static volatile usize this_tick = 0;
  static volatile usize ticks_left = 0;
  static volatile bool first_switch = true;

  if (ticks_left == 0) {
    // TODO: Purge the done processes?

    // Find a new thing to run
    usize lowest_run = 0;
    usize max_delta = 0;
    // FIXME: need range-based for :)
    for (usize i = 0; i < m_processes.len(); i++) {
      usize delta;
      // Check overflow
      if (m_processes[i].m_last_run > this_tick) {
        delta = m_processes[i].m_last_run - this_tick;
      } else {
        delta = this_tick - m_processes[i].m_last_run;
      }

      if (delta > max_delta) {
        if (i > 0 && i == m_current_process) {
          continue;
        }
        max_delta = delta;
        lowest_run = i;
      }
    }

    ticks_left = ticks_per_task;

    if (!first_switch) {
      // No reason to switch tasks if we can avoid it
      if (m_current_process == lowest_run) {
        m_processes[m_current_process].m_last_run = this_tick++;
        return;
      }

      m_processes[m_current_process].m_registers.update_from_frame(frame);
    } else {
      first_switch = false;
    }

    auto &proc = m_processes[lowest_run];
    _x86_set_page_directory(reinterpret_cast<u32>(proc.m_page_directory));

    // proc.push_return_address();
    new_esp = proc.m_registers.esp;
    new_eip = proc.m_registers.eip;

    proc.m_registers.prepare_frame(frame);
    frame->eip = reinterpret_cast<uintptr_t>(&_x86_task_switch);
    frame->eflags |= 0x200;
    proc.m_last_run = this_tick;
    m_current_process = lowest_run;
  } else {
    ticks_left--;
  }

  this_tick++;
}
} // namespace kernel
