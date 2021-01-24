#pragma once

#include <stddef.h>

// FIXME: Find out where this should go
#ifdef __IS_X86__
typedef __INT32_TYPE__ ssize_t;
#else
#error Expected x86
#endif

#ifdef __cplusplus
extern "C" {
#endif

long syscall(long number, ...);
ssize_t write(int fd, const void *buf, size_t bytes);

#ifdef __cplusplus
}
#endif
