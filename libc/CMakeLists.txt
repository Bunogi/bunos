set (
  LIBC_SOURCES
  ctype.cpp
  stdio.cpp
  stdlib.cpp
  string.cpp
)

add_library(c-kernel STATIC ${LIBC_SOURCES})
target_compile_definitions(c-kernel PRIVATE __IN_KERNEL__=1)

add_library(c.object STATIC ${LIBC_SOURCES})
add_custom_target(
  crt0.o
  command ${CMAKE_ASM_COMPILER} -c ${CMAKE_SOURCE_DIR}/libc/x86/crt0.s -o crt0.o
  DEPENDS ${CMAKE_SOURCE_DIR}/libc/x86/crt0.s)

add_custom_target(
  c
  COMMAND mkdir -p ${BUNOS_FS_ROOT_DIR}/usr/lib
  COMMAND cp libc.object.a ${BUNOS_FS_ROOT_DIR}/usr/lib/libc.a
  COMMAND cp crt0.o ${BUNOS_FS_ROOT_DIR}/usr/lib/crt0.o
  DEPENDS c.object crt0.o
)

file(GLOB LIBC_HEADERS *.h sys/*.h)
add_custom_target(
  install_libc_headers
  COMMAND mkdir -p ${BUNOS_INSTALL_DIR}/usr/include/sys
  COMMAND cp ${LIBC_HEADERS} ${BUNOS_INSTALL_DIR}/usr/include
  DEPENDS ${LIBC_HEADERS})
