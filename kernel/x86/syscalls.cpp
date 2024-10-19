#include <kernel/interrupts.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/syscalls.hpp>
#include <kernel/x86/interrupts.hpp>
#include <kernel/x86/syscalls.hpp>

namespace kernel::x86 {
namespace {
auto interrupt_handler(InterruptFrame *frame) -> bool {
  auto &proc = Scheduler::current_process();
  proc.start_syscall();

  Scheduler::yield(frame);
  return true;
}
} // namespace

constexpr u8 syscall_vector = 0x80;
void register_syscall_handler() {
  interrupts::register_handler(syscall_vector, interrupt_handler);
}

} // namespace kernel::x86
