#pragma once

#include <bustd/stddef.hpp>
#include <kernel/process.hpp>
#include <libc/sys/syscall.h>

namespace kernel {
typedef __bunos_syscall Syscall;

#define MAX_SYSCALL_ARG_COUNT 4

void init_syscalls();
i32 do_syscall(Process &proc, u32 code, u32 *parameters);

} // namespace kernel
