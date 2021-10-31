#pragma once

#define O_RDONLY (1 << 0)

#ifdef __cplusplus
extern "C" {
#endif
int open(const char *path, int oflags);
#ifdef __cplusplus
}
#endif
