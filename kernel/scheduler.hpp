#pragma once

#include <bustd/macros.hpp>
#include <bustd/vector.hpp>
#include <kernel/process.hpp>
#include <kernel/x86/interrupts.hpp>

namespace kernel {

class Scheduler {
public:
  static void init();
  static void spawn(Process &&p);
  static void run();
  void wake(interrupt::x86::InterruptFrame *frame);

private:
  bu::Vector<Process> m_processes;
  volatile usize m_ticks_left{0};
  volatile usize m_running_pid{0};
  volatile usize m_current_process{0};
};

} // namespace kernel
