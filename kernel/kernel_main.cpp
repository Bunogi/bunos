#include <bustd/assert.hpp>

#include <stdio.h>

#include "kprint.hpp"
#include "panic.hpp"
#include "x86/gdt.hpp"
#include "x86/interrupts.hpp"
#include "x86/io.hpp"
#include "x86/memory.hpp"

#include "tty/kerneloutputdevice.hpp"
#include <kernel/x86/interruptmanager.hpp>

extern "C" {
void kernel_main() {
  kernel::x86::io::ensure_ring0_only();
  kernel::memory::x86::setup_gdt();
  kernel::memory::x86::init_memory_management();

  // TODO: Allocate in the ::instance() function instead of here when we get
  // memory allocation
  kernel::interrupt::x86::initialize();
  kernel::interrupt::x86::InterruptManager manager;
  kernel::interrupt::x86::InterruptManager::init(&manager);

  kernel::tty::KernelOutputDevice print_device;
  kernel::print::init(&print_device);

  manager.enable_interrupts();

  printf("Welcome to Bunos 0.0-dev!\n");
  printf("Booting...\n");

  while (1) {
  }
  KERNEL_PANIC("Reached end of kernel_main");
}
}
