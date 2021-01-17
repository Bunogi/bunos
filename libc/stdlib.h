#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// FIXME: Should probably have an include file or something
typedef __SIZE_TYPE__ size_t;

void exit(int status);

void *malloc(size_t size);
void free(void *ptr);
void abort(void);
int atexit(void (*function)(void));

int atoi(const char *ptr);
long atol(const char *ptr);
long long atoll(const char *ptr);

#define atoq(...) atoll(__VA_ARGS__)

long strtol(const char *ptr, char **end, int base);
long long strtoll(const char *ptr, char **end, int base);

#ifdef __cplusplus
}
#endif
