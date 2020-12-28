#pragma once

#include <bustd/assert.hpp>
#include <bustd/stddef.hpp>

namespace kernel::vmem {
enum class ReservedRegion : u64 {
  Vga = 0xC0002000,
};

// Return the region size in 4KiB pages
constexpr usize reserved_region_size(const ReservedRegion region) {
  switch (region) {
  case ReservedRegion::Vga:
    return 1;
  }
  UNREACHABLE();
}
} // namespace kernel::vmem
