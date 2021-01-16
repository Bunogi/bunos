#include <bustd/bitfield.hpp>
#include <kernel/memory.hpp>
#include <kernel/physicalmalloc.hpp>
#include <kernel/x86/memory.hpp>

// TODO remove
#include <stdio.h>

extern "C" {
extern char _kernel_heap_end;
}

namespace kernel::pmem {
static bu::Bitfield<8192> s_allocations;
static const u64 s_allocatable_page_start =
    VirtualAddress(reinterpret_cast<uintptr_t>(&_kernel_heap_end))
        .to_linked_location()
        .get();

PhysicalPage::PhysicalPage(PhysicalAddress addr) : m_address(addr) {}
PhysicalAddress PhysicalPage::address() const { return m_address; }

// TODO: Parse memory map and stuff
void init() { ASSERT_EQ(s_allocatable_page_start & 0xFFF, 0); }

PhysicalPage allocate() {
  printf("[pmem] start: %p \n", s_allocatable_page_start);

  // TODO: This should allow you to use a range-based for
  for (usize i = 0; i < s_allocations.size(); i++) {
    if (!s_allocations.at(i)) {
      s_allocations.set(i, true);
      const auto address = i * PAGE_SIZE + s_allocatable_page_start;
      printf("[pmem] Allocated physical page at %p\n", address);
      return PhysicalPage(PhysicalAddress(address));
    }
  }
  KERNEL_PANIC("Out of physical pages to allocate!");
  return PhysicalPage(PhysicalAddress(0));
}

void deallocate(PhysicalPage page) {
  // They have to be page aligned
  ASSERT_EQ(page.address().get() & (PAGE_SIZE - 1), 0);
  ASSERT(page.address().get() >= s_allocatable_page_start);

  const auto as_index =
      (page.address().get() - s_allocatable_page_start) / PAGE_SIZE;
  ASSERT_EQ(s_allocations.at(as_index), true);
  s_allocations.set(as_index, false);
}
} // namespace kernel::pmem
