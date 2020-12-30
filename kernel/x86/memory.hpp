#pragma once

#include <bustd/stddef.hpp>

namespace kernel::memory::x86 {
constexpr u64 virt_to_phys_addr(u64 virt) { return virt - 0xC0000000; }

void init_memory_management();

// Allocate a physical page to use in the kernel, available to all kernel tasks.
// TODO: use a pair structure thing
void *map_kernel_memory(u32 page_count);
} // namespace kernel::memory::x86
