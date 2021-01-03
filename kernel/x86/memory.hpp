#pragma once

#include <bustd/stddef.hpp>
extern "C" {
// memory.s
extern void _x86_set_page_directory(u32 addr);
extern void _x86_refresh_page_directory();
}

namespace kernel::memory::x86 {
extern u32 kernel_page_table[1024] __attribute__((aligned(4096)));
extern u32 kernel_page_directory[1024] __attribute__((aligned(4096)));
constexpr u32 kernel_address_space_dir_index = 768;

constexpr u64 virt_to_phys_addr(u64 virt) { return virt - 0xC0000000; }

void init_memory_management();

// Allocate a physical page to use in the kernel, available to all kernel tasks.
// TODO: use a pair structure thing
void *map_kernel_memory(u32 page_count);
} // namespace kernel::memory::x86
