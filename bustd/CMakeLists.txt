add_library(bustd-kernel STATIC memory.cpp memory.hpp)
add_compile_definitions(__IN_KERNEL__=1)
