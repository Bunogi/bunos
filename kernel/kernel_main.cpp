#include "kprint.hpp"
#include "panic.hpp"
#include "x86/gdt.hpp"
#include "x86/interrupts.hpp"
#include "x86/io.hpp"
#include <bustd/assert.hpp>

#include "tty/kerneloutputdevice.hpp"

#include <stdio.h>

extern "C" {
void kernel_main() {
  kernel::x86::io::ensure_ring0_only();
  kernel::tty::KernelOutputDevice print_device;
  kernel::print::init(&print_device);

  printf("Welcome to Bunos 0.0-dev!\n");
  printf("Booting...\n");
  kernel::memory::x86::setup_gdt();
  kernel::interrupt::x86::initialize();

  KERNEL_PANIC("Reached end of kernel_main");
}
}
