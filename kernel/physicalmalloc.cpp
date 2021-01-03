#include <bustd/bitfield.hpp>
#include <kernel/physicalmalloc.hpp>
#include <kernel/x86/memory.hpp>

// TODO remove
#include <stdio.h>

extern "C" {
extern char _kernel_heap_end;
}

// TODO: Parse memory map and stuff
// TODO: This does not have to be heap-allocated when global constructors get
// called
static bu::Bitfield<8192> *s_allocations;
static u64 s_allocatable_page_start;
constexpr u64 physical_page_size = 0x1000;

namespace kernel::pmem {
PhysicalPage::PhysicalPage(u64 addr) : m_address(addr) {}
u64 PhysicalPage::address() const { return m_address; }

void init() {
  ASSERT_EQ(s_allocations, nullptr);
  s_allocations = new bu::Bitfield<8192>();
  // TODO: call global constructors, put this outside
  s_allocatable_page_start =
      kernel::x86::virt_to_phys_addr(reinterpret_cast<u64>(&_kernel_heap_end)) +
      physical_page_size;
  ASSERT_EQ(s_allocatable_page_start & 0xFFF, 0);
}

PhysicalPage allocate() {
  ASSERT_NE(s_allocations, nullptr);
  // printf("[pmem] start: %p \n", s_allocatable_page_start);
  // TODO: This should allow you to use a range-based for
  for (usize i = 0; i < s_allocations->size(); i++) {
    if (!s_allocations->at(i)) {
      s_allocations->set(i, true);
      const auto address = i * physical_page_size + s_allocatable_page_start;
      printf("[pmem] Allocated physical page at %p\n", address);
      return PhysicalPage(address);
    }
  }
  KERNEL_PANIC("Out of physical pages to allocate!");
  return PhysicalPage(0);
}
void deallocate(PhysicalPage page) {
  ASSERT_NE(s_allocations, nullptr);

  // They have to be page aligned
  ASSERT_EQ(page.address() & (physical_page_size - 1), 0);
  ASSERT(page.address() >= s_allocatable_page_start);

  const auto as_index =
      (page.address() - s_allocatable_page_start) / physical_page_size;
  ASSERT_EQ(s_allocations->at(as_index), true);
  s_allocations->set(as_index, false);
}
} // namespace kernel::pmem
