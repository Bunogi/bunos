#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t strlen(const char *);
void *strcpy(char *dest, const char *src);
void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *ptr, int value, size_t n);

#ifdef __cplusplus
}
#endif
