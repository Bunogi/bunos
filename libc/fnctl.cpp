#include <libc/errno.h>
#include <libc/fnctl.h>
#include <libc/sys/syscall.h>
#include <libc/unistd.h>

extern "C" {
int open(const char *const path, const int oflags) {
  const auto retval = syscall(SYS_OPEN, path, oflags);
  if (retval < 0) {
    errno = -retval;
    return -1;
  }
  return retval;
}
}
