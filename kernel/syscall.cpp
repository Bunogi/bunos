#include <bustd/stddef.hpp>
#include <kernel/syscall.hpp>
#include <libc/stdio.h>

extern "C" {
void do_syscall(usize num, usize arg1, usize arg2, usize arg3,
                kernel::Process *const target_proc) {
  kernel::SyscallArguments args(num, arg1, arg2, arg3);

  kernel::syscall_handler(args, target_proc);
}
}

namespace kernel {

void syscall_handler(const SyscallArguments &args, Process *const target_proc) {
  char buffer[512];
  sprintf(buffer, "SYSCALL %x: args %x, %x, %x, target: %p\n", args.get(0),
          args.get(1), args.get(2), args.get(3), target_proc);

  KERNEL_PANIC(buffer);
}
} // namespace kernel
