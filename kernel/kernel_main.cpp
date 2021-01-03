#include <bustd/assert.hpp>
#include <bustd/vector.hpp>
#include <kernel/interruptmanager.hpp>
#include <kernel/kmalloc.hpp>
#include <kernel/kprint.hpp>
#include <kernel/panic.hpp>
#include <kernel/physicalmalloc.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/timer.hpp>
#include <kernel/tty/kerneloutputdevice.hpp>
#include <kernel/x86/gdt.hpp>
#include <kernel/x86/interrupts.hpp>
#include <kernel/x86/io.hpp>
#include <kernel/x86/memory.hpp>
#include <kernel/x86/tty/vga.hpp>
#include <stdio.h>

extern "C" {
void kernel_main() {
  using namespace kernel;

  x86::tty::Vga early_printer;
  print::init(&early_printer);

  x86::set_io_permissions();
  x86::setup_gdt();
  x86::init_memory_management();
  malloc::Allocator allocator;
  pmem::init();

  // TODO: Allocate in the ::instance() function instead of here when we get
  // memory allocation
  x86::initialize_interrupts();
  InterruptManager manager;
  InterruptManager::init(&manager);

  printf("=== Switching to late print device ===\n");
  kernel::tty::KernelOutputDevice print_device(bu::move(early_printer));
  kernel::print::init(&print_device);

  kernel::Scheduler::init();

  kernel::timer::initialize();
  printf("Welcome to Bunos 0.0-dev!\n");
  printf("Running scheduler...\n");

  kernel::Scheduler::run();

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
