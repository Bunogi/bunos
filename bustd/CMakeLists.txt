add_library(bustd-kernel STATIC string_view.cpp)
add_compile_definitions(__IN_KERNEL__=1)
