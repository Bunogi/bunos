#pragma once

#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

long syscall(long number, ...);
ssize_t write(int fd, const void *buf, size_t bytes);
ssize_t read(int fd, void *buf, size_t bytes);
int close(int fd);

#ifdef __cplusplus
}
#endif
