#pragma once

#include <bustd/function.hpp>
#include <bustd/iteration.hpp>
#include <bustd/list.hpp>
#include <bustd/macros.hpp>
#include <bustd/stringview.hpp>
#include <kernel/process.hpp>
#include <kernel/x86/interrupts.hpp>
#include <libc/sys/types.h>

namespace kernel {

class Scheduler {
public:
  static void init();
  static void spawn(bu::StringView, void (*func)());
  static void run();
  void wake(x86::InterruptFrame *frame);
  static auto current_process() -> Process &;
  static void yield(x86::InterruptFrame *frame);

  // HACK: running tests
  static void run_next_test_proc(pid_t exited_pid);

private:
  Scheduler();
  void switch_task(pid_t pid, x86::InterruptFrame *frame, bool allow_update);
  auto get_next_process() -> usize;
  auto next_pid() -> pid_t;
  void spawn_test_processes();

  auto find_process(pid_t) -> Process &;

  pid_t m_next_pid{0};
  // Use a list to ensure the references returned by current_process() do not
  // shift in memory
  bu::List<Process> m_processes;

  // HACK: run tests, replace with proper system :)
  bu::List<Process> m_test_processes;
  pid_t m_current_test_process{0};

  pid_t m_current_process{0};

  bool m_first_switch{true};
  usize m_ticks_left{0};
  usize m_this_tick{0};
};

} // namespace kernel
