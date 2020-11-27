#include "kprint.hpp"
#include "panic.hpp"
#include "tty/i386/vga.hpp"

#include <bustd/memory.hpp>

extern "C" {
void kernel_main() {
  kernel::tty::Vga vga;
  kernel::print::init(&vga);

  kernel::print::println("Hello, world!");

  kernel::print::print("My cool number 0 is ");
  kernel::print::number(0);
  kernel::print::print("\nMy cool number 0xFF is ");
  kernel::print::number(0xFF);
  kernel::print::print("\nMy cool number 0x11223344 is ");
  kernel::print::number(0x11223344);

  kernel::print::println("\nReached end of kernel_main");
  kernel::panic::panic();
}
}
