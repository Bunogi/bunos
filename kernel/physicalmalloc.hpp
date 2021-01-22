#pragma once

#include <bustd/macros.hpp>
#include <bustd/stddef.hpp>
#include <kernel/memory.hpp>

namespace kernel {
void init_pmem();

PhysicalAddress allocate_physical_page();
void free_physical_page(PhysicalAddress addr);

} // namespace kernel
