#include "kprint.hpp"
#include "panic.hpp"
#include "x86/gdt.hpp"
#include "x86/tty/vga.hpp"

#include <stdio.h>

extern "C" {
void kernel_main() {
  kernel::tty::x86::Vga vga_dev;
  kernel::print::init(&vga_dev);
  kernel::memory::x86::setup_gdt();

  printf("Reached end of kernel_main\n");
  kpanic();
}
}
