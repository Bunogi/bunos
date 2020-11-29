#include "kprint.hpp"
#include "panic.hpp"
#include "tty/iodevice.hpp"

#include <stdio.h>

extern "C" {
void kernel_main() {
  kernel::tty::x86::Vga vga_dev;
  kernel::print::init(&vga_dev);
  // kprintln("Hello, world!");

  // kprint("My cool number 0 is ");
  // kprint_number(0);
  // kprint("\nMy cool number 0xFF is ");
  // kprint_number(0xFF);
  // kprint("\nMy cool number 0x11223344 is ");
  // kprint_number(0x11223344);

  printf("Reached end of kernel_main\n");
  kpanic();
}
}
