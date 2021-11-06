#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// FIXME: Should probably have an include file or something
typedef __SIZE_TYPE__ size_t;

void exit(int status);

void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void free(void *ptr);

#ifdef __cplusplus
[[noreturn]] void abort(void);
#else
void abort(void);
#endif

int atexit(void (*function)(void));

int atoi(const char *ptr);
long atol(const char *ptr);
long long atoll(const char *ptr);

#define atoq(...) atoll(__VA_ARGS__)

long strtol(const char *ptr, char **end, int base);
long long strtoll(const char *ptr, char **end, int base);

// Not certain whetther this is technically correct or not, but the manpage of
// stdlib.h says it is allowed. Libgcc needed this.
#include <stdint.h>

#ifdef __cplusplus
}
#endif
