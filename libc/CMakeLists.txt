add_library(libc-kernel STATIC string.cpp)
add_compile_definitions(__IN_KERNEL__=1)
