cmake_minimum_required(VERSION 3.18.2)

set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)
set(CMAKE_TOOLCHAIN_FILE cross.cmake)

project(bunos CXX ASM)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${CMAKE_SOURCE_DIR}/kernel/x86/linker.ld")

set(CMAKE_CXX_STANDARD 17)

#add_custom_target(
#  OUTPUT boot.o
#  COMMAND ${ASSEMBLER} ${CMAKE_SOURCE_DIR}/kernel/x86/boot.s -o boot.o
#  DEPENDS ${CMAKE_SOURCE_DIR}/kernel/x86/boot.s
#)

#add_custom_target(
#  boot.o
#  COMMAND ${ASSEMBLER} ${CMAKE_SOURCE_DIR}/kernel/x86/boot.s -o boot.o
#  SOURCES "${CMAKE_SOURCE_DIR}/kernel/x86/boot.s")
#set_source_files_properties(boot.o PROPERTIES GENERATED true)

# TODO: Setup crtbegin.o and crtend.o

#add_custom_command(
#  OUTPUT crtbegin.o crtend.o
#  COMMAND sh -c "cp \$(${CMAKE_CXX_COMPILER} -print-file-name=crtbegin.o) ."
#  COMMAND sh -c "cp \$(${CMAKE_CXX_COMPILER} -print-file-name=crtend.o) ."
#  VERBATIM
#)

# standard QEMU flags for use with everything
set(QEMU_FLAGS -m 4G -no-reboot -serial stdio)
add_custom_target(
  run
  COMMAND qemu-system-i386 -kernel kernel.bin ${QEMU_FLAGS}
  DEPENDS kernel.bin
)
add_custom_target(
  image
  COMMAND mkdir -p isodir/boot/grub
  COMMAND cp kernel.bin isodir/boot
  COMMAND cp ${CMAKE_SOURCE_DIR}/grub.cfg isodir/boot/grub/grub.cfg
  COMMAND grub-mkrescue isodir -o image.iso
  VERBATIM
  DEPENDS kernel.bin
)
add_custom_target(
  run_image
  COMMAND qemu-system-i386 -cdrom image.iso ${QEMU_FLAGS}
  DEPENDS image
)
add_custom_target(
  run_debug
  COMMAND qemu-system-i386 -kernel kernel.bin -s -S ${QEMU_FLAGS} -d int
  DEPENDS kernel.bin
)

add_executable(
  kernel.bin
  kernel/kernel_main.cpp
  kernel/kprint.cpp
  kernel/panic.cpp
  kernel/tty/kerneloutputdevice.cpp
  kernel/utils.cpp
  kernel/x86/boot.s
  kernel/x86/gdt.cpp
  kernel/x86/gdt.s
  kernel/x86/io.cpp
  kernel/x86/io.s
  kernel/x86/tty/serial.cpp
  kernel/x86/tty/vga.cpp
)
#add_dependencies(kernel.bin boot.o)

include_directories(${CMAKE_SOURCE_DIR} libc) #TODO: remove libc from this when able

add_subdirectory(bustd)
add_subdirectory(libc)

set_target_properties(kernel.bin PROPERTIES LINK_DEPENDS
  "${CMAKE_SOURCE_DIR}/kernel/x86/linker.ld"
)
target_link_libraries(kernel.bin -lgcc c-kernel bustd-kernel)
target_include_directories(kernel.bin PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_definitions(kernel.bin PRIVATE __IN_KERNEL__=1 __IS_X86__=1)
