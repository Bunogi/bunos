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

# standard QEMU flags for use with everything
set(QEMU_FLAGS -m 4G -serial stdio)
set(DISK_IMAGE -drive file=../disk.img,format=raw)

add_custom_target(
  run
  COMMAND qemu-system-i386 -kernel kernel.bin -no-reboot ${QEMU_FLAGS} ${DISK_IMAGE}
  DEPENDS kernel.bin image
)

add_custom_target(
  kernel.sym
  ALL
  COMMAND mkdir -p ${BUNOS_INSTALL_DIR}/boot/ && nm -n kernel.bin | c++filt > ${BUNOS_INSTALL_DIR}/boot/kernel.sym
  VERBATIM
  DEPENDS kernel.bin
)

add_custom_target(
  run_debug
  COMMAND qemu-system-i386 -kernel kernel.bin -s -S ${QEMU_FLAGS} -d int ${DISK_IMAGE}
  DEPENDS kernel.bin image
)

add_library(kernel.object
  STATIC
  cxxabi.cpp
  debugsymbols.cpp
  elfreader.cpp
  filesystem/ext2.cpp
  filesystem/inodeindex.cpp
  filesystem/vfs.cpp
  interruptguard.cpp
  interrupts.cpp
  kernel_main.cpp
  kmalloc.cpp
  kprint.cpp
  panic.cpp
  physicalmalloc.cpp
  process.cpp
  process/keyboardfile.cpp
  scheduler.cpp
  spinlock.cpp
  syscalls.cpp
  test_kmalloc.cpp
  timer.cpp
  tty/kerneloutputdevice.cpp
  x86/boot.S
  x86/gdt.cpp
  x86/gdt.s
  x86/interrupts.cpp
  x86/interrupts.S
  x86/io.cpp
  x86/io.s
  x86/memory.cpp
  x86/memory.s
  x86/pagemapguard.cpp
  x86/paging.cpp
  x86/pata.cpp
  x86/pic.cpp
  x86/pit.cpp
  x86/ps2.cpp
  x86/ps2_keyboard.cpp
  x86/scheduler.s
  x86/syscalls.cpp
  x86/tty/serial.cpp
  x86/tty/vga.cpp
)

set(KERNEL_LINK_SCRIPT "${CMAKE_SOURCE_DIR}/kernel/x86/linker.ld")
set_target_properties(kernel.object PROPERTIES COMPILE_FLAGS "${KERNEL_BUILD_FLAGS}")
target_compile_definitions(kernel.object PRIVATE __IN_KERNEL__=1 __IS_X86__=1)
target_compile_options(kernel.object PRIVATE -fstack-protector-all -fsanitize=undefined)

add_custom_command(
  OUTPUT crtbegin.o crtend.o
  COMMAND sh -c "cp \$(${CMAKE_CXX_COMPILER} -print-file-name=crtbegin.o) ."
  COMMAND sh -c "cp \$(${CMAKE_CXX_COMPILER} -print-file-name=crtend.o) ."
  VERBATIM
)

add_custom_target(
  crti.o
  COMMAND ${CMAKE_ASM_COMPILER} -c ${CMAKE_SOURCE_DIR}/kernel/x86/crti.s -o crti.o
  DEPENDS ${CMAKE_SOURCE_DIR}/kernel/x86/crti.s
)

add_custom_target(
  crtn.o
  COMMAND ${CMAKE_ASM_COMPILER} -c ${CMAKE_SOURCE_DIR}/kernel/x86/crtn.s -o crtn.o
  DEPENDS ${CMAKE_SOURCE_DIR}/kernel/x86/crtn.s
)

set(KERNEL_LINK_LIBS
  crti.o
  crtbegin.o
  -L. -lkernel.object
  -L../libc -lc-kernel
  -L../bustd -lbustd-kernel
  -L../libraries/libubsan -lubsan-kernel
  -L../libraries/libstacktrace -lstacktrace-kernel
  -lgcc
  crtend.o
  crtn.o)

set(KERNEL_CXX_FLAGS ${CMAKE_CXX_FLAGS})
separate_arguments(KERNEL_CXX_FLAGS)

add_custom_target(
  kernel.bin
  COMMAND ${CMAKE_CXX_COMPILER} ${KERNEL_CXX_FLAGS} ${KERNEL_LINK_LIBS} -T ${KERNEL_LINK_SCRIPT} -nostdlib -o kernel.bin
  DEPENDS
    kernel.object
    ${KERNEL_LINK_SCRIPT}
    crti.o
    crtn.o
    crtbegin.o
    crtend.o
    bustd-kernel
    c-kernel
    ubsan-kernel
    stacktrace-kernel
  VERBATIM
)
