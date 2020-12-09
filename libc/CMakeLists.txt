set (LIBC_SOURCES string.cpp stdio.cpp)
add_library(c-kernel STATIC ${LIBC_SOURCES})
target_compile_definitions(c-kernel PRIVATE __IN_KERNEL__=1 __IS_X86__=1)
