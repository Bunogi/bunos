#pragma once

#include <stddef.h>

#ifdef __cplusplus

extern "C" {
#endif
size_t strlen(const char *);
void *memcpy(void *dest, const void *src, size_t n);

#ifdef __cplusplus
}
#endif
