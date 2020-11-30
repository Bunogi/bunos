cmake_minimum_required(VERSION 3.18)

set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)
project(bunos)

set(CMAKE_BUILD_TYPE Debug)

set(WARNING_FLAGS "-Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Og -g")
set(CMAKE_SYSROOT "${CMAKE_SOURCE_DIR}/sysroot")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS} -fno-exceptions -ffreestanding -fno-rtti -nostdlib")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${CMAKE_SOURCE_DIR}/kernel/x86/linker.ld -Xlinker -Map=test.map")

set(TOOLCHAIN_PREFIX "${CMAKE_SOURCE_DIR}/toolchain/prefix/bin/i686-elf")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PREFIX}-g++")
set(CMAKE_C_COMPILER "${TOOLCHAIN_PREFIX}-gcc")
set(ASSEMBLER "${TOOLCHAIN_PREFIX}-as")

set(CMAKE_CXX_STANDARD 17)

add_custom_command(
  OUTPUT boot.o
  COMMAND ${ASSEMBLER} ${CMAKE_SOURCE_DIR}/kernel/x86/boot.s -o boot.o
  DEPENDS ${CMAKE_SOURCE_DIR}/kernel/x86/boot.s
)

#add_custom_target(
#  boot.o
#  COMMAND ${ASSEMBLER} ${CMAKE_SOURCE_DIR}/kernel/boot.s -o boot.o
#  SOURCES "${CMAKE_SOURCE_DIR}/kernel/boot.s")
#set_source_files_properties(boot.o PROPERTIES GENERATED true)

# TODO: Setup crtbegin.o and crtend.o

#add_custom_command(
#  OUTPUT crtbegin.o crtend.o
#  COMMAND sh -c "cp \$(${CMAKE_CXX_COMPILER} -print-file-name=crtbegin.o) ."
#  COMMAND sh -c "cp \$(${CMAKE_CXX_COMPILER} -print-file-name=crtend.o) ."
#  VERBATIM
#)

add_library(boot STATIC boot.o)
set_target_properties(boot PROPERTIES LINKER_LANGUAGE CXX)

add_custom_target(
  run
  COMMAND qemu-system-i386 -kernel kernel.bin -m 4G
  DEPENDS kernel.bin
)
add_custom_target(
  run_debug
  COMMAND qemu-system-i386 -kernel kernel.bin -m 4G -s -S
  DEPENDS kernel.bin
)

add_executable(
  kernel.bin
  kernel/kernel_main.cpp
  kernel/kprint.cpp
  kernel/panic.cpp
  kernel/utils.cpp
  kernel/x86/tty/vga.cpp
)

include_directories(${CMAKE_SOURCE_DIR} libc) #TODO: remove libc from this when able

add_dependencies(kernel.bin boot)

add_subdirectory(bustd)
add_subdirectory(libc)

set_target_properties(kernel.bin PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/kernel/x86/linker.ld)
target_link_libraries(kernel.bin -lgcc boot libc-kernel)
target_include_directories(kernel.bin PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_definitions(kernel.bin PRIVATE __IN_KERNEL__=1 __IS_X86__=1)
