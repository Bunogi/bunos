#include <bustd/assert.hpp>
#include <bustd/vector.hpp>
#include <kernel/debugsymbols.hpp>
#include <kernel/filesystem/ext2.hpp>
#include <kernel/filesystem/vfs.hpp>
#include <kernel/interrupts.hpp>
#include <kernel/kmalloc.hpp>
#include <kernel/kprint.hpp>
#include <kernel/panic.hpp>
#include <kernel/physicalmalloc.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/syscalls.hpp>
#include <kernel/timer.hpp>
#include <kernel/tty/kerneloutputdevice.hpp>
#include <kernel/x86/gdt.hpp>
#include <kernel/x86/io.hpp>
#include <kernel/x86/memory.hpp>
#include <kernel/x86/pata.hpp>
#include <kernel/x86/ps2.hpp>
#include <kernel/x86/tty/vga.hpp>
#include <stdio.h>

extern "C" {
extern void _init();
}

extern "C" {
void kernel_main() {
  using namespace kernel;
  _init();

  x86::tty::Vga early_printer;
  print::init(&early_printer);

  x86::set_io_permissions();
  x86::setup_gdt();
  x86::init_memory_management();
  malloc::Allocator allocator;
  init_pmem();

  interrupts::init();

  printf("=== Switching to late print device ===\n");
  tty::KernelOutputDevice print_device(bu::move(early_printer));
  print::init(&print_device);

  kernel::Scheduler::init();

  timer::initialize();
  printf("Welcome to Bunos 0.0-dev!\n");

  x86::initialize_pata();

  Vfs::instance().mount(bu::OwnedPtr<IFileSystem>(new filesystem::Ext2()));

  printf("Main: Disk has %u sectors\n", x86::disk_sector_count());
  load_debug_symbols();

  x86::init_ps2_controller();
  init_syscalls();
  printf("Running scheduler...\n");

  kernel::Scheduler::run();

  KERNEL_PANIC("Reached end of kernel_main");
}
}
