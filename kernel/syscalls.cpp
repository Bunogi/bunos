#include <kernel/syscalls.hpp>
#include <kernel/x86/syscalls.hpp>
#include <libc/errno.h>
#include <libc/stdio.h>
#include <libc/sys/syscall.h>

namespace kernel {
void init_syscalls() { x86::register_syscall_handler(); }

} // namespace kernel
