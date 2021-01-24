#include <kernel/syscalls.hpp>
#include <kernel/x86/syscalls.hpp>
#include <libc/errno.h>
#include <libc/stdio.h>
#include <libc/sys/syscall.h>

namespace kernel {
void init_syscalls() { x86::register_syscall_handler(); }

i32 do_syscall(Process &proc, u32 code, u32 *parameters) {
  switch (code) {
  case SYS_EXIT:
    proc.sys_exit(parameters[0]);
    return 0;
  case SYS_WRITE:
    return proc.sys_write(parameters[0],
                          reinterpret_cast<const void *>(parameters[1]),
                          parameters[2]);
  default:
    printf("[syscall] Unknown syscall code %u\n", code);
    // Invalid syscall :(
    return -ENOSYS;
  }
}
} // namespace kernel
