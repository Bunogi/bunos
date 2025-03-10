#include <bustd/assert.hpp>
#include <bustd/stddef.hpp>
#include <libc/errno.h>
#include <libc/stdlib.h>
#include <libc/sys/syscall.h>
#include <libc/unistd.h>
#include <stdarg.h>

#ifdef __IS_X86__
extern "C" {
i32 _x86_syscall_no_args(u32 code);
i32 _x86_syscall_one_arg(u32 code, u32 arg);
i32 _x86_syscall_two_args(u32 code, u32 arg1, u32 arg2);
i32 _x86_syscall_three_args(u32 code, u32 arg1, u32 arg2, u32 arg3);
}
#endif

long syscall(long code, ...) {
  if (code >= __SYSCALL_COUNT) {
    errno = ENOSYS;
    return 1;
  }
  u8 argcount = 0;

  switch (code) {
  case SYS_CLOSE:
  case SYS_EXIT:
    argcount = 1;
    break;
  case SYS_OPEN:
    argcount = 2;
    break;
  case SYS_WRITE:
  case SYS_READ:
    argcount = 3;
    break;
  default:
    // FIXME: set errno
    UNREACHABLE();
    return -1;
  }

  u32 args[4];

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
  const auto retval = syscall(SYS_WRITE, fd, buf, bytes);
  if (retval < 0) {
    errno = -retval;
    return -1;
  }
  return retval;
}

int close(const int fd) {
  const auto retval = syscall(SYS_CLOSE, fd);
  if (retval < 0) {
    errno = -retval;
    return -1;
  }
  return 0;
}

ssize_t read(int fd, void *buf, size_t bytes) {
  const auto retval = syscall(SYS_READ, fd, buf, bytes);
  if (retval < 0) {
    errno = -retval;
    return -1;
  }
  return retval;
}
