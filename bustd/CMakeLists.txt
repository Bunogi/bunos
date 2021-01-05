set(BUSTD_SOURCES stringview.cpp)

bunos_lib(NAME bustd KERNEL_SPECIFIC_BUILD_REQUIRED SOURCES ${BUSTD_SOURCES})

add_subdirectory(test)
