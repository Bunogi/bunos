#include <kernel/interrupts.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/syscalls.hpp>
#include <kernel/x86/interrupts.hpp>
#include <kernel/x86/syscalls.hpp>

namespace kernel::x86 {
namespace {
bool interrupt_handler(InterruptFrame *frame) {
  auto &proc = Scheduler::current_process();
  u32 parameters[MAX_SYSCALL_ARG_COUNT];
  const u32 code = frame->eax;
  parameters[0] = frame->ebx;
  parameters[1] = frame->ecx;
  parameters[2] = frame->edx;
  parameters[3] = frame->edi;
  const auto result = do_syscall(proc, code, parameters);
  proc.return_from_syscall(result);
  Scheduler::yield(frame);
  return true;
}
} // namespace

constexpr u8 syscall_vector = 0x80;
void register_syscall_handler() {
  interrupts::register_handler(syscall_vector, interrupt_handler);
}

} // namespace kernel::x86
