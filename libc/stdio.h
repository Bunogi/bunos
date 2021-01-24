#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// FIXME: We should split these up somehow so that they are nice to include
// where they should be
typedef __SIZE_TYPE__ size_t;
#define SEEK_SET 1

#define EOF -1

int sprintf(char *str, const char *format, ...);
int printf(const char *format, ...);
int puts(const char *s);

typedef void FILE;
extern FILE *stderr, *stdout, *stdin;

int fflush(FILE *stream);
int fprintf(FILE *stream, const char *format, ...);
char *getenv(const char *name);

#ifdef __cplusplus
}
#endif
