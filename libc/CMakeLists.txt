add_library(libc-kernel STATIC string.cpp stdio.cpp)
add_compile_definitions(__IN_KERNEL__=1)
