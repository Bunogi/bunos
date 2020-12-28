#include "memory.hpp"

#include <bustd/assert.hpp>
#include <kernel/x86/virtualmemorymap.hpp>
#include <stdio.h>

extern "C" {
// From boot.S, we can use these
extern char _kernel_nonwritable_start, _kernel_nonwritable_end,
    _kernel_writable_start, _kernel_writable_end, _kernel_heap_start,
    _kernel_heap_end;

// memory.s
extern void _x86_refresh_page_directory(u32 addr);

extern u32 boot_page_directory[1024];
extern u32 boot_page_table1[1024];
}

namespace kernel::memory::x86 {
constexpr u64 to_physical_address(u64 addr) { return addr - 0xC0000000; }
} // namespace kernel::memory::x86
namespace {
namespace Local {

class PageDirectoryEntry {
public:
  bool present : 1;
  bool read_write : 1;
  bool user : 1;
  bool write_through : 1;
  bool cache_disable : 1;
  bool accessed : 1;
  bool zero : 1; // must be zero in a valid entry
  bool is_large : 1;
  bool global : 1;
  u8 available : 3;
  u32 page_table_address;

  u32 as_u32() {
    ASSERT_EQ(page_table_address & 0x00000FFF, 0);
    u32 out = page_table_address & 0xFFFFF000;
    out |= static_cast<u8>(present) << 0;
    out |= static_cast<u8>(read_write) << 1;
    out |= static_cast<u8>(user) << 2;
    out |= static_cast<u8>(write_through) << 3;
    out |= static_cast<u8>(cache_disable) << 4;
    out |= static_cast<u8>(accessed) << 5;
    // out |= static_cast<u8>(zero) << 6; // always zero anyway
    out |= static_cast<u8>(is_large) << 7;
    out |= static_cast<u8>(global) << 8;
    out |= static_cast<u8>(available) << 9;
    return out;
  }
};

class PageTableEntry {
public:
  bool present : 1;
  bool read_write : 1;
  bool user : 1;
  bool write_through : 1;
  bool cached : 1;
  bool accessed : 1;
  bool dirty : 1;
  bool zero : 1; // must be zero in a valid entry
  bool global : 1;
  u8 available : 3;
  u32 physical_page_address;
  u32 as_u32() {
    ASSERT_EQ(physical_page_address & 0x00000FFF, 0);
    u32 out = (physical_page_address & 0xFFFFF000);
    out |= static_cast<u8>(present) << 0;
    out |= static_cast<u8>(read_write) << 1;
    out |= static_cast<u8>(user) << 2;
    out |= static_cast<u8>(write_through) << 3;
    out |= static_cast<u8>(cached) << 4;
    out |= static_cast<u8>(accessed) << 5;
    // out |= static_cast<u8>(zero) << 6; // always zero anyway
    out |= static_cast<u8>(dirty) << 7;
    out |= static_cast<u8>(global) << 8;
    out |= static_cast<u8>(available) << 9;
    return out;
  }
};

static u32 kernel_page_table[1024] __attribute__((aligned(4096)));
static u32 kernel_page_directory[1024] __attribute__((aligned(4096)));

void init_kernel_area(PageTableEntry &&table_settings, const u32 start,
                      const u32 stop) {

  const u32 start_page = start / 0x1000, end_page = stop / 0x1000 + 1;
  ASSERT_EQ(start & 0xFFF, 0);

  // we have to do end_offset - start_offset pages + 1 to account for overrun
  uintptr_t physical_page;
  for (physical_page = start_page; physical_page < end_page; physical_page++) {
    const auto physical_address = physical_page * 4096;
    table_settings.physical_page_address = physical_address;
    table_settings.present = true;

    // We can do this because the address translation is linear.
    const auto index = physical_page % 0x1000;
    kernel_page_table[index] = table_settings.as_u32();
  }
}

void init_nonwritable_kernel_sections() {
  const u32 start_offset = kernel::memory::x86::to_physical_address(
      reinterpret_cast<uintptr_t>(&_kernel_nonwritable_start));
  const u32 end_offset = kernel::memory::x86::to_physical_address(
      reinterpret_cast<uintptr_t>(&_kernel_nonwritable_end));

  PageTableEntry entry{};
  init_kernel_area(bu::move(entry), start_offset, end_offset);

  // Must be page aligned
}

void init_writable_kernel_sections() {
  const u32 start_offset = kernel::memory::x86::to_physical_address(
      reinterpret_cast<uintptr_t>(&_kernel_writable_start));
  const u32 end_offset = kernel::memory::x86::to_physical_address(
      reinterpret_cast<uintptr_t>(&_kernel_writable_end));

  PageTableEntry entry{}; // All fields zero
  entry.read_write = true;
  init_kernel_area(bu::move(entry), start_offset, end_offset);
}

void init_kernel_heap_section() {
  const u32 start_offset = kernel::memory::x86::to_physical_address(
      reinterpret_cast<uintptr_t>(&_kernel_heap_start));
  const u32 end_offset = kernel::memory::x86::to_physical_address(
      reinterpret_cast<uintptr_t>(&_kernel_heap_end));

  PageTableEntry entry{}; // All fields zero
  entry.read_write = true;
  init_kernel_area(bu::move(entry), start_offset, end_offset);
}

void init_other_sections() {
  // VGA -> 0xxxx2000
  PageTableEntry entry{};
  entry.physical_page_address = 0xB8000;
  entry.present = true;
  entry.read_write = true;

  constexpr u32 index =
      kernel::memory::x86::to_physical_address(
          static_cast<u64>(kernel::vmem::ReservedRegion::Vga)) /
      0x1000;
  kernel_page_table[index] = entry.as_u32();
}

void reinit_page_directory() {
  // Should not overlap
  ASSERT(&_kernel_nonwritable_end <= &_kernel_writable_start);

  init_nonwritable_kernel_sections();
  init_writable_kernel_sections();
  init_kernel_heap_section();
  init_other_sections();

  uintptr_t table_address = reinterpret_cast<uintptr_t>(&kernel_page_table[0]);
  table_address -= 0xC0000000;

  PageDirectoryEntry entry{};
  entry.page_table_address = table_address;
  entry.present = true;
  entry.read_write = true;

  kernel_page_directory[768] = entry.as_u32();

  u32 address = kernel::memory::x86::to_physical_address(
      reinterpret_cast<uintptr_t>(&kernel_page_directory));
  _x86_refresh_page_directory(address);
}

} // namespace Local
} // namespace

namespace kernel::memory::x86 {
void init_memory_management() {
  Local::reinit_page_directory();

  printf("Re-mapped to proper paging setup\n");
}

} // namespace kernel::memory::x86
