#pragma once

#include <bustd/function.hpp>
#include <bustd/iteration.hpp>
#include <bustd/list.hpp>
#include <bustd/macros.hpp>
#include <kernel/process.hpp>
#include <kernel/x86/interrupts.hpp>
#include <libc/sys/types.h>

namespace kernel {

class Scheduler {
public:
  static void init();
  static void spawn(void (*func)());
  static void run();
  void wake(x86::InterruptFrame *frame);
  static Process &current_process();
  static void yield(x86::InterruptFrame *frame);

private:
  Scheduler();
  void switch_task(usize proc_index, x86::InterruptFrame *frame,
                   bool allow_update);
  usize get_next_process();
  pid_t next_pid();

  pid_t m_next_pid{0};
  // Use a list to ensure the references returned by current_process() do not
  // shift in memory
  bu::List<Process> m_processes;
  // FIXME: This ought to be a pointer
  usize m_current_process{0};

  bool m_first_switch{true};
  usize m_ticks_left{0};
  usize m_this_tick{0};
};

} // namespace kernel
