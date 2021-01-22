#include <bustd/assert.hpp>
#include <kernel/syscalls.hpp>
#include <libc/errno.h>
#include <libc/stdlib.h>
#include <libc/sys/syscall.h>
#include <libc/unistd.h>
#include <stdarg.h>

#ifdef __IS_X86__
extern "C" {
i32 _x86_do_syscall(u32 code, u32 *args, u32 count);
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
  default:
    // FIXME: set errno
    abort();
    return -1;
  }

  ASSERT_NE(argcount, 0);

  u32 args[MAX_SYSCALL_ARG_COUNT];

  va_list arg_list;
  va_start(arg_list, code);
  for (u8 i = 0; i < argcount; i++) {
    args[i] = va_arg(arg_list, u32);
  }
  va_end(arg_list);

  const auto retval = _x86_do_syscall(code, args, argcount);
  if (retval < 0) {
    errno = -retval;
  }
  return retval;
}
