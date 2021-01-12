#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// FIXME: Should probably have an include file or something
typedef __SIZE_TYPE__ size_t;

void exit(int status);

void *malloc(size_t size);
void free(void *ptr);
#ifdef __cplusplus
}
#endif
