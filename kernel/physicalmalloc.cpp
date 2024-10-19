#include <bustd/bitfield.hpp>
#include <kernel/memory.hpp>
#include <kernel/physicalmalloc.hpp>
#include <kernel/x86/memory.hpp>

//#define DEBUG_MEMORY
#ifdef DEBUG_MEMORY
#include <stdio.h>
#define DEBUG_PRINTF(s, ...) printf("[pmem]" s, __VA_ARGS__)
#else
#define DEBUG_PRINTF(s, ...)
#endif

extern "C" {
extern char _kernel_heap_end;
}

namespace kernel {
static bu::Bitfield<8192> s_allocations;
static const u64 s_allocatable_page_start =
    VirtualAddress(reinterpret_cast<uintptr_t>(&_kernel_heap_end))
        .to_linked_location()
        .get();

// TODO: Parse memory map and stuff
void init_pmem() { ASSERT_EQ(s_allocatable_page_start & 0xFFF, 0); }

auto allocate_physical_page() -> PhysicalAddress {
  DEBUG_PRINTF("start: %p \n", s_allocatable_page_start);

  // TODO: This should allow you to use a range-based for
  for (usize i = 0; i < s_allocations.size(); i++) {
    if (!s_allocations.at(i)) {
      s_allocations.set(i, true);
      const auto address = i * PAGE_SIZE + s_allocatable_page_start;
      DEBUG_PRINTF("Allocated physical page at %p\n", address);
      return PhysicalAddress(address);
    }
  }
  KERNEL_PANIC("Out of physical pages to allocate!");
  return PhysicalAddress(0);
}

void free_physical_page(PhysicalAddress page) {
  DEBUG_PRINTF("Supposed to delete %pP\n", page.get());
  // They have to be page aligned
  ASSERT_EQ(page.get() & (PAGE_SIZE - 1), 0);
  ASSERT(page.get() >= s_allocatable_page_start);

  const auto as_index = (page.get() - s_allocatable_page_start) / PAGE_SIZE;
  ASSERT_EQ(s_allocations.at(as_index), true);
  s_allocations.set(as_index, false);
}

} // namespace kernel
