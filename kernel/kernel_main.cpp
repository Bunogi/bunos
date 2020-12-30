#include <bustd/assert.hpp>

#include <stdio.h>

#include "kprint.hpp"
#include "panic.hpp"
#include "x86/gdt.hpp"
#include "x86/interrupts.hpp"
#include "x86/io.hpp"
#include "x86/memory.hpp"

#include "tty/kerneloutputdevice.hpp"
#include <kernel/kmalloc.hpp>
#include <kernel/timer.hpp>
#include <kernel/x86/interruptmanager.hpp>
#include <kernel/x86/physicalmalloc.hpp>
#include <kernel/x86/tty/vga.hpp>

extern "C" {
void kernel_main() {
  kernel::tty::x86::Vga early_printer;
  kernel::print::init(&early_printer);

  kernel::x86::io::ensure_ring0_only();
  kernel::memory::x86::setup_gdt();
  kernel::memory::x86::init_memory_management();
  kernel::malloc::Allocator allocator;
  kernel::pmem::init();

  // TODO: Allocate in the ::instance() function instead of here when we get
  // memory allocation
  kernel::interrupt::x86::initialize();
  kernel::interrupt::x86::InterruptManager manager;
  kernel::interrupt::x86::InterruptManager::init(&manager);

  printf("=== Switching to late print device ===\n");
  kernel::tty::KernelOutputDevice print_device(bu::move(early_printer));
  kernel::print::init(&print_device);

  kernel::timer::initialize();

  manager.enable_interrupts();

  printf("Welcome to Bunos 0.0-dev!\n");
  printf("Booting...\n");

  printf("Nothing more to do, relaxing here for a while...   ");
  const char *const animation = "\\|/-\\|/";
  u16 i = 0;
  while (1) {
    if (i >= sizeof animation) {
      i = 0;
    }
    printf("\b%c", animation[i++]);
    kernel::timer::delay(1000);
  }
  KERNEL_PANIC("Reached end of kernel_main");
}
}
