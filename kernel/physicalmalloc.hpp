#pragma once

#include <bustd/stddef.hpp>
#include <kernel/memory.hpp>

namespace kernel::pmem {
void init();

PhysicalAddress allocate();
void deallocate(PhysicalAddress page);

} // namespace kernel::pmem
