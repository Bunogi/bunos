#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define __BUNOS_ENUMERATE_SYSCALL(o) o(CLOSE) o(EXIT) o(OPEN) o(READ) o(WRITE)

#define o(s) SYS_##s,
enum __bunos_syscall { __BUNOS_ENUMERATE_SYSCALL(o) __SYSCALL_COUNT };
#undef o

#ifdef __cplusplus
}
#endif
