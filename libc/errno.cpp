#include <bustd/assert.hpp>
#include <libc/errno.h>
#include <libc/stdio.h>
#include <libc/string.h>

namespace __bunos_libc {
const char *get_error_string(int errnum) {
  switch (errnum) {
  case 0:
    return "Success";
  case EINVAL:
    return "Invalid argument";
  case ENOSYS:
    return "No such syscall";
  case EBADF:
    return "Bad file descriptor";
  case EACCES:
    return "Permission denied";
  default:
    // FIXME: Should output to debug only
    printf("[libc] %s(): unknown errno %u\n",
           static_cast<const char *>(__func__), errnum);
    return "Unknown error";
  }
}
} // namespace __bunos_libc

extern "C" {

int errno;
}
