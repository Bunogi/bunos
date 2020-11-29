#include "kprint.hpp"
#include "panic.hpp"
#include "x86/tty/vga.hpp"

#include <stdio.h>

extern "C" {
void kernel_main() {
  kernel::tty::x86::Vga vga_dev;
  kernel::print::init(&vga_dev);

  printf("Unsigned print: %u\n", 300000);

  printf("Signed print: %i\n", 3000);
  printf("Signed print: %d\n", -3000);
  printf("Pointer print: %p\n", &vga_dev);
  printf("String print: '%s'\n", "hello, world");
  printf("percentage print: %%\n");
  printf("char print: %c%c\n", 'c', ':');

  printf("Reached end of kernel_main\n");
  kpanic();
}
}
