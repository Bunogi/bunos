#include "kprint.hpp"
#include "panic.hpp"
#include "x86/gdt.hpp"
#include "x86/io.hpp"

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

  printf("Reached end of kernel_main\n");
  kpanic();
}
}
