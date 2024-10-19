#pragma once

#include <bustd/assert.hpp>
#include <bustd/stddef.hpp>
#include <kernel/memory.hpp>

namespace kernel::vmem {

namespace reserved {
// FIXME: maybe a future refactoring should have a Region structure instead?

// For temporarily mapping individual pages
constexpr VirtualAddress Temp(0xC0001000);
// Vga memory
constexpr VirtualAddress Vga(0xC0002000);
} // namespace reserved

// Return the region size in 4KiB pages
constexpr auto reserved_region_size(const VirtualAddress region) -> usize {
  if (region == reserved::Temp || region == reserved::Vga) {
    return 1;
  }
  UNREACHABLE();
  return 0;
}
} // namespace kernel::vmem
