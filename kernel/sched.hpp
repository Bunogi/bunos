#pragma once

#include <bustd/list.hpp>
#include <bustd/macros.hpp>
#include <kernel/process.hpp>
#include <kernel/task/arch.hpp>
#include <kernel/x86/interrupts.hpp>

namespace kernel::sched {
void run();

class Scheduler {
  BU_NOCOPY(Scheduler);
  BU_NOMOVE(Scheduler);

public:
  Scheduler();

  static auto systick_event(x86::InterruptFrame *) -> void;
  static auto syscall_event(x86::InterruptFrame *) -> void;

  void run();

private:
  static Scheduler *s_scheduler;
  void load_test_processes();

  void handle_systick(x86::InterruptFrame *frame);
  void handle_syscall(x86::InterruptFrame *frame);
  void reschedule(x86::InterruptFrame *frame);

  pid_t m_next_pid{1};
  struct SchedInfo {
    usize run_for_ticks{0};
    Process proc;
  };
  bu::List<SchedInfo> m_processes;
  bu::List<SchedInfo>::LoopingIterator m_current_proc;
};

} // namespace kernel::sched
