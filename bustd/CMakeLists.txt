set(BUSTD_SOURCES string_view.cpp)

add_library(bustd-kernel STATIC ${BUSTD_SOURCES})
target_compile_definitions(bustd-kernel PRIVATE __IN_KERNEL__=1)
set_target_properties(bustd-kernel PROPERTIES COMPILE_FLAGS "${KERNEL_BUILD_FLAGS}")

add_library(bustd STATIC ${BUSTD_SOURCES})

add_subdirectory(test)
