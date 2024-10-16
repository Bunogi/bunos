#include <bustd/assert.hpp>
#include <kernel/memory.hpp>
#include <kernel/physicalmalloc.hpp>
#include <kernel/process.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/timer.hpp>
#include <kernel/x86/memory.hpp>
#include <kernel/x86/pagemapguard.hpp>
#include <kernel/x86/paging.hpp>
#include <kernel/x86/virtualmemorymap.hpp>
#include <stdio.h>

extern "C" {
// From boot.S, we can use these
extern char _kernel_start, _kernel_nonwritable_start, _kernel_nonwritable_end,
    _kernel_writable_start, _kernel_writable_end, _kernel_heap_start,
    _kernel_heap_end;

extern u32 boot_page_directory[1024];
extern u32 boot_page_table1[1024];
}

namespace {
using namespace kernel;
using namespace kernel::x86;
VirtualAddress virtual_address_from_indices(const u16 page_directory_index,
                                            const u16 page_table_index = 0) {
  constexpr u32 dir_entries = 1024;
  constexpr u32 bytes_per_entry = 0x1000; // 4M
  const u32 from_directory =
      page_directory_index * dir_entries * bytes_per_entry;
  const u32 from_table = page_table_index * 0x1000;
  return VirtualAddress(from_directory + from_table);
}

u16 page_dir_index_from_addr(const VirtualAddress &addr) {
  constexpr u32 dir_entries = 1024;
  constexpr u32 kilobytes_per_entry = 0x1000; // 4M
  const u32 to_check = addr.get() & 0xFFFFF000;

  return to_check / (dir_entries * kilobytes_per_entry);
}

u16 page_table_index_from_addr(const VirtualAddress &addr) {
  constexpr u32 bytes_per_entry = 0x1000; // 4K

  constexpr u32 page_dir_entries = 1024;
  constexpr u32 kilobytes_per_page_dir_entry = 0x1000;
  const u32 to_check =
      addr.get() % (page_dir_entries * kilobytes_per_page_dir_entry);

  return to_check / bytes_per_entry;
}

void init_kernel_area(PageTableEntry &&table_settings, const u32 start,
                      const u32 stop) {

  const u32 start_page = start / 0x1000, end_page = stop / 0x1000 + 1;
  ASSERT_EQ(start & 0xFFF, 0);

  // we have to do end_offset - start_offset pages + 1 to account for overrun
  for (uintptr_t physical_page = start_page; physical_page < end_page;
       physical_page++) {
    table_settings.page_address = kernel::PhysicalAddress(physical_page * 4096);
    table_settings.present = true;

    // We can do this because the address translation is linear.
    const auto index = physical_page % 0x1000;
    kernel::x86::kernel_page_table[index] = table_settings.as_u32();
  }
}

void init_nonwritable_kernel_sections() {
  const u32 start_offset = kernel::VirtualAddress(&_kernel_nonwritable_start)
                               .to_linked_location()
                               .get();
  const u32 end_offset = kernel::VirtualAddress(&_kernel_nonwritable_end)
                             .to_linked_location()
                             .get();

  PageTableEntry entry{};
  init_kernel_area(bu::move(entry), start_offset, end_offset);

  // Must be page aligned
}

void init_writable_kernel_sections() {
  const u32 start_offset = kernel::VirtualAddress(&_kernel_writable_start)
                               .to_linked_location()
                               .get();

  const u32 end_offset =
      kernel::VirtualAddress(&_kernel_writable_end).to_linked_location().get();

  PageTableEntry entry{}; // All fields zero
  entry.read_write = true;
  init_kernel_area(bu::move(entry), start_offset, end_offset);
}

void init_kernel_heap_section() {
  const u32 start_offset =
      kernel::VirtualAddress(&_kernel_heap_start).to_linked_location().get();
  const u32 end_offset =
      kernel::VirtualAddress(&_kernel_heap_end).to_linked_location().get();

  PageTableEntry entry{}; // All fields zero
  entry.read_write = true;
  init_kernel_area(bu::move(entry), start_offset, end_offset);
}

void init_other_sections() {
  // VGA -> 0xxxx2000
  PageTableEntry entry{};
  entry.page_address = kernel::PhysicalAddress(0xB8000);
  entry.present = true;
  entry.read_write = true;

  constexpr u32 index =
      kernel::vmem::reserved::Vga.to_linked_location().get() / 0x1000;
  kernel::x86::kernel_page_table[index] = entry.as_u32();
}

void reinit_page_directory() {
  // Should not overlap
  ASSERT(&_kernel_nonwritable_end <= &_kernel_writable_start);

  init_nonwritable_kernel_sections();
  init_writable_kernel_sections();
  init_kernel_heap_section();
  init_other_sections();

  uintptr_t table_address =
      reinterpret_cast<uintptr_t>(&kernel::x86::kernel_page_table[0]);
  table_address -= 0xC0000000;

  PageDirectoryEntry entry{};
  entry.page_table_address = table_address;
  entry.present = true;
  entry.read_write = true;
  kernel::x86::kernel_page_directory
      [kernel::x86::kernel_address_space_dir_index] = entry.as_u32();

  u32 address = kernel::VirtualAddress(&kernel::x86::kernel_page_directory)
                    .to_linked_location()
                    .get();
  _x86_set_page_directory(address);
}

} // namespace

namespace kernel::x86 {
u32 kernel_page_table[1024] __attribute__((aligned(4096)));
u32 kernel_page_directory[1024] __attribute__((aligned(4096)));

void init_memory_management() {
  reinit_page_directory();

  printf("Re-initialized paging\n");
}

VirtualAddress map_kernel_memory(u32 continous_page_count) {
  ASSERT(continous_page_count > 0);
  // TODO: do this for every kernel thread
  // FIXME: This way of detecting free entries won't work if we have explicitly
  // mapped some memory as not present

  // If we are out of space in this page directory entry,
  // create another one, and map that in at the kernel address plus whatever.
  // Use that as the starting point otherwise, use the end of kernel label as a
  // starting point.
  u16 page_directory_index = 0;
  u16 page_table_index = 0;
  const auto kernel_end_as_table_index =
      page_table_index_from_addr(VirtualAddress(&_kernel_heap_end));
  ASSERT(kernel_end_as_table_index < 1024);

  u16 free_in_a_row = 0;
  for (u16 i = kernel_end_as_table_index; i < 1024; i++) {
    const auto entry =
        PageTableEntry::from_u32(kernel::x86::kernel_page_table[i]);
    if (!entry.present) {
      free_in_a_row++;
      if (free_in_a_row == continous_page_count) {
        page_directory_index = kernel::x86::kernel_address_space_dir_index;
        ASSERT(i > continous_page_count);
        page_table_index = i - continous_page_count;
        break;
      }
    } else {
      free_in_a_row = 0;
    }
  }

  // Did not find free space in the first page dir entry, have to look further
  // ahead
  if (page_directory_index == 0) {
    if (free_in_a_row != 0) {
      puts("[vmem] Would probably be able to allocate across page-directory "
           "boundaries");
    }

    for (u16 i = kernel::x86::kernel_address_space_dir_index + 1; i < 1024;
         i++) {
      const auto entry =
          PageDirectoryEntry::from_u32(kernel::x86::kernel_page_directory[i]);
      if (!entry.present) {
        TODO("Need to create a new page directory entry");
      }

      // FIXME: We should support using addresses across page-directory
      // boundaries for multipage allocs
      free_in_a_row = 0;
      for (u16 j = 0; j < 0x1000; j++) {
        const u32 *table =
            static_cast<u32 *>(virtual_address_from_indices(i, j).ptr());
        const PageMapGuard guard((kernel::PhysicalAddress(
            reinterpret_cast<kernel::PhysicalAddress::Type>(table))));
        const auto entry = PageTableEntry::from_u32(
            *reinterpret_cast<u32 *>(guard.mapped_address()));
        if (!entry.present) {
          free_in_a_row++;
          if (free_in_a_row == continous_page_count) {
            page_directory_index = i;
            ASSERT(j > continous_page_count);
            page_table_index = j - continous_page_count;
            goto found_entry;
          }
        } else {
          free_in_a_row = 0;
        }
      }
    }
  }
found_entry:

  if (page_directory_index == 0) {
    // FIXME: This is a job for an OOM killer :^)
    KERNEL_PANIC("Out of virtual kernel address space");
  }

  ASSERT(page_directory_index < 1024);
  ASSERT(page_table_index + continous_page_count < 1024);

  const auto entry = PageDirectoryEntry::from_u32(
      kernel::x86::kernel_page_directory[page_directory_index]);
  u32 table = entry.page_table_address;

  const PageMapGuard guard((PhysicalAddress(table)));
  u32 *const page_table = reinterpret_cast<u32 *>(guard.mapped_address());

  for (u32 i = page_table_index; i < page_table_index + continous_page_count;
       i++) {

    PageTableEntry new_entry{};
    // FIXME: Have some way to de-allocate afterwards
    const auto physical_page = kernel::allocate_physical_page();
    new_entry.page_address = physical_page;
    new_entry.read_write = true;
    new_entry.present = true;

    page_table[i] = new_entry.as_u32();
  }

  auto *const retval =
      virtual_address_from_indices(page_directory_index, page_table_index)
          .ptr();
  return VirtualAddress(retval);
}

// FIXME: Needs to return whether a page was actually mapped
bool map_user_memory(Process &process, VirtualAddress at) {
  const auto pd_entry = page_dir_index_from_addr(at);
  const auto pt_entry = page_table_index_from_addr(at);
  ASSERT(at.ptr() < &_kernel_start);

  auto page_dir = process.page_dir();
  PhysicalAddress page_table;
  {
    // Page dir of the process is always initialized
    const PageMapGuard guard((page_dir));
    u32 *const dir = static_cast<u32 *>(guard.mapped_address());
    auto entry = PageDirectoryEntry::from_u32(dir[pd_entry]);
    if (!entry.present) {
      entry = PageDirectoryEntry();

      auto allocated = allocate_physical_page();
      entry.page_table_address = allocated.get();
      process.take_page_table_page(bu::move(allocated));

      entry.user = true;
      entry.read_write = true;
      entry.present = true;
      dir[pd_entry] = entry.as_u32();
    }
    page_table = PhysicalAddress(entry.page_table_address);
  }

  {
    const PageMapGuard guard((page_table));
    u32 *const table = static_cast<u32 *>(guard.mapped_address());

    auto physical_page = allocate_physical_page();
    PageTableEntry new_entry{};
    new_entry.page_address = physical_page;
    process.take_memory_page(bu::move(physical_page));
    new_entry.read_write = true;
    new_entry.user = true;
    new_entry.present = true;

    table[pt_entry] = new_entry.as_u32();
  }
  return true;
}

void set_user_mem_no_write(Process &process, VirtualAddress at) {
  const auto pd_entry = page_dir_index_from_addr(at);
  const auto pt_entry = page_table_index_from_addr(at);
  PhysicalAddress page_table;
  {
    const auto page_dir = process.page_dir();
    const PageMapGuard guard((page_dir));
    auto *const dir = static_cast<u32 *>(guard.mapped_address());
    const auto entry = PageDirectoryEntry::from_u32(dir[pd_entry]);
    ASSERT(entry.present);
    page_table = PhysicalAddress(entry.page_table_address);
  }

  {
    const PageMapGuard guard((page_table));
    u32 *const table = static_cast<u32 *>(guard.mapped_address());
    auto entry = PageTableEntry::from_u32(table[pt_entry]);
    ASSERT(entry.present);
    entry.read_write = false;
    table[pt_entry] = entry.as_u32();
  }
}

} // namespace kernel::x86
