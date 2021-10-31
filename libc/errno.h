#pragma once

#ifdef __cplusplus
namespace __bunos_libc {
// can't modify the string returned by strerror even in perror, so this has to
// be separate
const char *get_error_string(int errnum);
} // namespace __bunos_libc

extern "C" {
#endif

// FIXME: Should be thread-local
extern int errno;

char *strerror(int errnum);

#define EINVAL 1
#define ENOSYS 2
#define EBADF 3
#define EACCES 4

#ifdef __cplusplus
}
#endif
