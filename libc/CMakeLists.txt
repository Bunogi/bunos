set (LIBC_SOURCES string.cpp stdio.cpp)
if (NOT BUNOS_BUILD_HOST)
    add_library(c-kernel STATIC ${LIBC_SOURCES})
    target_compile_definitions(c-kernel PRIVATE __IN_KERNEL__=1)
    set_target_properties(c-kernel PROPERTIES COMPILE_FLAGS "${KERNEL_BUILD_FLAGS}")
endif()

#add_library(c STATIC ${LIBC_SOURCES})
