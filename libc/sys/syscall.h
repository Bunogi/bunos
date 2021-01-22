#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define __BUNOS_ENUMERATE_SYSCALL(o) o(EXIT)

#define o(s) SYS_##s,
enum __bunos_syscall { __BUNOS_ENUMERATE_SYSCALL(o) __SYSCALL_COUNT };

#ifdef __cplusplus
}
#endif
