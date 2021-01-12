#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t strlen(const char *);
char *strcpy(char *dest, const char *src);
int strcmp(const char *lhs, const char *rhs);
int strncmp(const char *lhs, const char *rhs, size_t n);

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *ptr, int value, size_t n);
int memcmp(const void *lhs, const void *rhs, size_t count);

#ifdef __cplusplus
}
#endif
