#include <bustd/assert.hpp>
#include <kernel/timer.hpp>
#include <kernel/x86/interruptmanager.hpp>
#include <kernel/x86/memory.hpp>
#include <kernel/x86/physicalmalloc.hpp>
#include <kernel/x86/virtualmemorymap.hpp>
#include <stdio.h>

extern "C" {
// From boot.S, we can use these
extern char _kernel_nonwritable_start, _kernel_nonwritable_end,
    _kernel_writable_start, _kernel_writable_end, _kernel_heap_start,
    _kernel_heap_end;

extern u32 boot_page_directory[1024];
extern u32 boot_page_table1[1024];
}

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

  u32 as_u32() const {
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

  static PageDirectoryEntry from_u32(u32 from) {
    PageDirectoryEntry out{};
    for (u8 i = 0; i < 9; i++) {
      switch (from & (1 << i)) {
      case 0:
        break;
      case 0x001:
        out.present = true;
        break;
      case 0x002:
        out.read_write = true;
        break;
      case 0x004:
        out.user = true;
        break;
      case 0x008:
        out.write_through = true;
        break;
      case 0x010:
        out.cache_disable = true;
        break;
      case 0x020:
        out.accessed = true;
        break;
      case 0x040:
        out.zero = true;
        break;
      case 0x080:
        out.is_large = true;
        break;
      case 0x100:
        out.global = true;
        break;
      default:
        UNREACHABLE();
      }
    }
    out.available = (from >> 9) & 0x7;
    out.page_table_address = from & 0xFFFFF000;
    return out;
  }
};

class PageTableEntry {
public:
  bool present : 1;
  bool read_write : 1;
  bool user : 1;
  bool write_through : 1;
  bool cache_disable : 1;
  bool accessed : 1;
  bool dirty : 1;
  bool zero : 1; // must be zero in a valid entry
  bool global : 1;
  u8 available : 3;
  u32 physical_page_address;

  u32 as_u32() const {
    ASSERT_EQ(physical_page_address & 0x00000FFF, 0);
    u32 out = (physical_page_address & 0xFFFFF000);
    out |= static_cast<u8>(present) << 0;
    out |= static_cast<u8>(read_write) << 1;
    out |= static_cast<u8>(user) << 2;
    out |= static_cast<u8>(write_through) << 3;
    out |= static_cast<u8>(cache_disable) << 4;
    out |= static_cast<u8>(accessed) << 5;
    // out |= static_cast<u8>(zero) << 6; // always zero anyway
    out |= static_cast<u8>(dirty) << 7;
    out |= static_cast<u8>(global) << 8;
    out |= static_cast<u8>(available) << 9;
    return out;
  }

  static PageTableEntry from_u32(u32 from) {
    PageTableEntry out{};
    for (u8 i = 0; i < 9; i++) {
      switch (from & (1 << i)) {
      case 0:
        break;
      case 0x001:
        out.present = true;
        break;
      case 0x002:
        out.read_write = true;
        break;
      case 0x004:
        out.user = true;
        break;
      case 0x008:
        out.write_through = true;
        break;
      case 0x010:
        out.cache_disable = true;
        break;
      case 0x020:
        out.accessed = true;
        break;
      case 0x040:
        out.dirty = true;
        break;
      case 0x080:
        out.zero = true;
        break;
      case 0x100:
        out.global = true;
        break;
      default:
        UNREACHABLE();
      }
    }
    out.available = (from >> 9) & 0x7;
    out.physical_page_address = from & 0xFFFFF000;
    return out;
  }
};

u32 virtual_address_from_indices(const u16 page_directory_index,
                                 const u16 page_table_index = 0) {
  constexpr u32 dir_entries = 1024;
  constexpr u32 bytes_per_entry = 0x1000; // 4M
  const u32 from_directory =
      page_directory_index * dir_entries * bytes_per_entry;
  const u32 from_table = page_table_index * 0x1000;
  return from_directory + from_table;
}

u16 page_dir_index_from_virtual_addr(const u32 addr) {
  constexpr u32 dir_entries = 1024;
  constexpr u32 bytes_per_entry = 0x1000; // 4M
  const u32 to_check = addr & 0xFFFFF000;

  return to_check / (dir_entries * bytes_per_entry);
}

// TODO: bustd should have some generic guard type to make stuff like this
// easier
class TemporaryPageMapGuard {
public:
  TemporaryPageMapGuard(const u64 physical_address)
      : m_guard(kernel::interrupt::x86::InterruptManager::instance()
                    ->disable_interrupts_guarded()) {
    // FIXME: use spinlock?
    if (s_m_in_use) {
      auto *instance = kernel::interrupt::x86::InterruptManager::instance();
      instance->enable_interrupts();
      while (s_m_in_use) {
        kernel::timer::delay(10);
      }
      s_m_in_use = true;
      instance->disable_interrupts();
    }
    s_m_in_use = true;

    PageTableEntry entry{};
    entry.physical_page_address = physical_address;
    entry.present = true;
    entry.read_write = true;
    // FIXME: Do to all threads
    constexpr u32 index =
        kernel::memory::x86::virt_to_phys_addr(
            static_cast<u64>(kernel::vmem::ReservedRegion::Temp)) /
        0x1000;
    kernel::memory::x86::kernel_page_table[index] = entry.as_u32();
    _x86_refresh_page_directory();
  }

  ~TemporaryPageMapGuard() {
    ASSERT(s_m_in_use);
    // FIXME: do for all threads
    constexpr u32 index =
        kernel::memory::x86::virt_to_phys_addr(
            static_cast<u64>(kernel::vmem::ReservedRegion::Temp)) /
        0x1000;
    PageTableEntry entry{};
    entry.present = false;
    kernel::memory::x86::kernel_page_table[index] = entry.as_u32();
    _x86_refresh_page_directory();
    s_m_in_use = false;
  }

  void *mapped_address() const {
    return reinterpret_cast<void *>(kernel::vmem::ReservedRegion::Temp);
  }

private:
  kernel::interrupt::x86::InterruptManager::InterruptGuard m_guard;
  static volatile bool s_m_in_use;
};

volatile bool TemporaryPageMapGuard::s_m_in_use{false};

u16 page_table_index_from_virtual_addr(const u32 addr) { return addr / 0x1000; }

void init_kernel_area(PageTableEntry &&table_settings, const u32 start,
                      const u32 stop) {

  const u32 start_page = start / 0x1000, end_page = stop / 0x1000 + 1;
  ASSERT_EQ(start & 0xFFF, 0);

  // we have to do end_offset - start_offset pages + 1 to account for overrun
  for (uintptr_t physical_page = start_page; physical_page < end_page;
       physical_page++) {
    const auto physical_address = physical_page * 4096;
    table_settings.physical_page_address = physical_address;
    table_settings.present = true;

    // We can do this because the address translation is linear.
    const auto index = physical_page % 0x1000;
    kernel::memory::x86::kernel_page_table[index] = table_settings.as_u32();
  }
}

void init_nonwritable_kernel_sections() {
  const u32 start_offset = kernel::memory::x86::virt_to_phys_addr(
      reinterpret_cast<uintptr_t>(&_kernel_nonwritable_start));
  const u32 end_offset = kernel::memory::x86::virt_to_phys_addr(
      reinterpret_cast<uintptr_t>(&_kernel_nonwritable_end));

  PageTableEntry entry{};
  init_kernel_area(bu::move(entry), start_offset, end_offset);

  // Must be page aligned
}

void init_writable_kernel_sections() {
  const u32 start_offset = kernel::memory::x86::virt_to_phys_addr(
      reinterpret_cast<uintptr_t>(&_kernel_writable_start));
  const u32 end_offset = kernel::memory::x86::virt_to_phys_addr(
      reinterpret_cast<uintptr_t>(&_kernel_writable_end));

  PageTableEntry entry{}; // All fields zero
  entry.read_write = true;
  init_kernel_area(bu::move(entry), start_offset, end_offset);
}

void init_kernel_heap_section() {
  const u32 start_offset = kernel::memory::x86::virt_to_phys_addr(
      reinterpret_cast<uintptr_t>(&_kernel_heap_start));
  const u32 end_offset = kernel::memory::x86::virt_to_phys_addr(
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

  constexpr u32 index = kernel::memory::x86::virt_to_phys_addr(static_cast<u64>(
                            kernel::vmem::ReservedRegion::Vga)) /
                        0x1000;
  kernel::memory::x86::kernel_page_table[index] = entry.as_u32();
}

void reinit_page_directory() {
  // Should not overlap
  ASSERT(&_kernel_nonwritable_end <= &_kernel_writable_start);

  init_nonwritable_kernel_sections();
  init_writable_kernel_sections();
  init_kernel_heap_section();
  init_other_sections();

  uintptr_t table_address =
      reinterpret_cast<uintptr_t>(&kernel::memory::x86::kernel_page_table[0]);
  table_address -= 0xC0000000;

  PageDirectoryEntry entry{};
  entry.page_table_address = table_address;
  entry.present = true;
  entry.read_write = true;
  kernel::memory::x86::kernel_page_directory
      [kernel::memory::x86::kernel_address_space_dir_index] = entry.as_u32();

  u32 address = kernel::memory::x86::virt_to_phys_addr(
      reinterpret_cast<uintptr_t>(&kernel::memory::x86::kernel_page_directory));
  _x86_set_page_directory(address);
}

} // namespace Local
} // namespace

namespace kernel::memory::x86 {
u32 kernel_page_table[1024] __attribute__((aligned(4096)));
u32 kernel_page_directory[1024] __attribute__((aligned(4096)));

void init_memory_management() {
  Local::reinit_page_directory();

  printf("Re-initialized paging\n");
}

// FIXME: This should be able to allocate several pages at once
void *map_kernel_memory(u32 page_count) {
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
      Local::page_table_index_from_virtual_addr(
          reinterpret_cast<uintptr_t>(&_kernel_heap_end));
  ASSERT(kernel_end_as_table_index < 1024);
  // reinterpret_cast<u64>(&_kernel_heap_end) / 1024;
  for (u16 i = kernel_end_as_table_index; i < 1024; i++) {
    const auto entry = Local::PageTableEntry::from_u32(
        kernel::memory::x86::kernel_page_table[i]);
    if (!entry.present) {
      page_directory_index =
          kernel::memory::x86::kernel_address_space_dir_index;
      page_table_index = i;
      break;
    }
  }

  // Did not find free space in the first page dir entry, have to look further
  // ahead
  if (page_directory_index == 0) {
    for (u16 i = kernel::memory::x86::kernel_address_space_dir_index + 1;
         i < 1024; i++) {
      const auto entry = Local::PageDirectoryEntry::from_u32(
          kernel::memory::x86::kernel_page_directory[i]);
      if (!entry.present) {
        TODO("Need to create a new page directory entry");
      }

      for (u16 j = 0; j < 0x1000; j++) {
        const u32 *table =
            reinterpret_cast<u32 *>(Local::virtual_address_from_indices(i, j));
        const Local::TemporaryPageMapGuard guard(
            reinterpret_cast<uintptr_t>(table));
        const auto entry = Local::PageTableEntry::from_u32(
            *reinterpret_cast<u32 *>(guard.mapped_address()));
        if (!entry.present) {
          page_directory_index = i;
          page_table_index = j;
          goto found_entry;
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
  ASSERT(page_table_index < 1024);
  printf("Found room at PD[%u], PT[%u], addr: %p\n", page_directory_index,
         page_table_index,
         Local::virtual_address_from_indices(page_directory_index,
                                             page_table_index));

  const auto entry = Local::PageDirectoryEntry::from_u32(
      kernel::memory::x86::kernel_page_directory[page_directory_index]);
  u32 table = entry.page_table_address;

  Local::PageTableEntry new_entry{};
  // FIXME: Have some way to de-allocate afterwards
  const auto physical_page = kernel::pmem::allocate();
  new_entry.physical_page_address = physical_page.address();
  new_entry.read_write = true;
  new_entry.present = true;

  // Temporarily map this table in memory instead of trying to find it mapped
  // somewhere.
  {
    const Local::TemporaryPageMapGuard guard(static_cast<u64>(table));
    u32 *const mapped = reinterpret_cast<u32 *>(guard.mapped_address());
    mapped[page_table_index] = new_entry.as_u32();
  }

  auto *const retval =
      reinterpret_cast<void *>(Local::virtual_address_from_indices(
          page_directory_index, page_table_index));
  printf("Successfully allocated kernel page at %p\n", retval);
  return retval;
}

} // namespace kernel::memory::x86
