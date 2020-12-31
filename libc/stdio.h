#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int sprintf(char *str, const char *format, ...);
int printf(const char *format, ...);
int puts(const char *s);

#ifdef __cplusplus
}
#endif
