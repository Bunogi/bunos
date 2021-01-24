#include <bustd/assert.hpp>
#include <kernel/syscalls.hpp>
#include <libc/errno.h>
#include <libc/stdlib.h>
#include <libc/sys/syscall.h>
#include <libc/unistd.h>
#include <stdarg.h>

#ifdef __IS_X86__
extern "C" {
i32 _x86_syscall_no_args(i32 code);
i32 _x86_syscall_one_arg(i32 code, i32 arg);
i32 _x86_syscall_two_args(i32 code, i32 arg1, i32 arg2);
i32 _x86_syscall_three_args(i32 code, i32 arg1, i32 arg2, i32 arg3);
}
#endif

long syscall(long code, ...) {
  if (code > __SYSCALL_COUNT) {
    abort();
  }
  kernel::Syscall call = static_cast<kernel::Syscall>(code);
  u8 argcount = 0;

  switch (call) {
  case SYS_EXIT:
    argcount = 1;
    break;
  case SYS_WRITE:
    argcount = 3;
    break;
  default:
    // FIXME: set errno
    // abort();
    return -1;
  }

  u32 args[MAX_SYSCALL_ARG_COUNT];

  va_list arg_list;
  va_start(arg_list, code);
  for (u8 i = 0; i < argcount; i++) {
    args[i] = va_arg(arg_list, u32);
  }
  va_end(arg_list);

  i32 retval = 0;
  switch (argcount) {
  case 0:
    retval = _x86_syscall_no_args(code);
    break;
  case 1:
    retval = _x86_syscall_one_arg(code, args[0]);
    break;
  case 2:
    retval = _x86_syscall_two_args(code, args[0], args[1]);
    break;
  case 3:
    retval = _x86_syscall_three_args(code, args[0], args[1], args[2]);
    break;
  default:
    UNREACHABLE();
  }

  if (retval < 0) {
    errno = -retval;
  }
  return retval;
}

ssize_t write(int fd, const void *buf, size_t bytes) {
  return syscall(SYS_WRITE, fd, buf, bytes);
}
