bunos_lib(
  NAME ubsan KERNEL_SPECIFIC_BUILD_REQUIRED HOST_DISABLE
  SOURCES
    abort.cpp
    type_mismatch.cpp
    ubsan.cpp
)
