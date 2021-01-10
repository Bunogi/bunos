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
set(QEMU_FLAGS -m 4G -serial stdio)
set(DISK_IMAGE -drive file=../disk.img,format=raw)

add_custom_target(
  run
  COMMAND qemu-system-i386 -kernel kernel.bin -no-reboot ${QEMU_FLAGS} ${DISK_IMAGE}
  DEPENDS kernel.bin image
)

add_custom_target(
  run_debug
  COMMAND qemu-system-i386 -kernel kernel.bin -s -S ${QEMU_FLAGS} -d int ${DISK_IMAGE}
  DEPENDS kernel.bin
)

add_executable(
  kernel.bin
  interruptmanager.cpp
  kernel_main.cpp
  kmalloc.cpp
  kprint.cpp
  panic.cpp
  physicalmalloc.cpp
  process.cpp
  scheduler.cpp
  timer.cpp
  filesystem/ext2.cpp
  tty/kerneloutputdevice.cpp
  utils.cpp
  x86/boot.s
  x86/gdt.cpp
  x86/gdt.s
  x86/interrupts.S
  x86/interrupts.cpp
  x86/io.cpp
  x86/io.s
  x86/memory.cpp
  x86/memory.s
  x86/pata.cpp
  x86/pic.cpp
  x86/pit.cpp
  x86/scheduler.s
  x86/tty/serial.cpp
  x86/tty/vga.cpp
)
#add_dependencies(kernel.bin boot.o)

set(KERNEL_LINK_SCRIPT "${CMAKE_SOURCE_DIR}/kernel/x86/linker.ld")
set_target_properties(kernel.bin PROPERTIES LINK_DEPENDS ${KERNEL_LINK_SCRIPT})
set_target_properties(kernel.bin PROPERTIES COMPILE_FLAGS "${KERNEL_BUILD_FLAGS}")

target_link_options(kernel.bin PRIVATE -T ${KERNEL_LINK_SCRIPT} -nostdlib)
target_link_libraries(kernel.bin -lgcc c-kernel bustd-kernel)
target_compile_definitions(kernel.bin PRIVATE __IN_KERNEL__=1 __IS_X86__=1)
