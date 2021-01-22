set (
  LIBC_SOURCES
  ctype.cpp
  stdio.cpp
  stdlib.cpp
  string.cpp
)

set(
  NONKERNEL_LIBC_SOURCES
  # FIXME: There are probably more, maybe there should be a neater split of what's available in the kernel and not?
  unistd.cpp
  errno.cpp
  x86/syscall.s
)

#FIXME: When userspace stuff is "done", these should not be part of kernel libc.
add_library(c-kernel STATIC ${LIBC_SOURCES} ${NONKERNEL_LIBC_SOURCES})
target_compile_definitions(c-kernel PRIVATE __IN_KERNEL__=1)

add_library(c.object STATIC ${LIBC_SOURCES} ${NONKERNEL_LIBC_SOURCES})
add_custom_target(
  crt0.o
  command ${CMAKE_ASM_COMPILER} -c ${CMAKE_SOURCE_DIR}/libc/x86/crt0.s -o crt0.o
  DEPENDS ${CMAKE_SOURCE_DIR}/libc/x86/crt0.s)

add_custom_target(
  c
  COMMAND ${CMAKE_SOURCE_DIR}/copy_libc_headers.sh > /dev/null
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
