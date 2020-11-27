#include "kprint.hpp"
#include "panic.hpp"
#include "tty/i386/vga.hpp"

#include <bustd/memory.hpp>

extern "C" {
void kernel_main() {
  kernel::tty::Vga vga;
  kernel::print::init(&vga);

  kprintln("Hello, world!");

  kprint("My cool number 0 is ");
  kprint_number(0);
  kprint("\nMy cool number 0xFF is ");
  kprint_number(0xFF);
  kprint("\nMy cool number 0x11223344 is ");
  kprint_number(0x11223344);

  kprintln("\nReached end of kernel_main");
  kpanic();
}
}
