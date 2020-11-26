#include "tty/i386/vga.hpp"

#include <bustd/memory.hpp>

extern "C" {
void kernel_main() {
  kernel::tty::Vga vga;
  vga.println("WEEEEEE");

  for (usize i = 0; i < 25; i++) {
    char i_as_num = i + '0';
    vga.print("Hullo world ");
    vga.putchar(i_as_num);
    vga.println("");
  }
}
}
