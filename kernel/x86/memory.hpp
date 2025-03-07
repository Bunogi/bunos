#pragma once

#include <bustd/stddef.hpp>
#include <kernel/memory.hpp>
#include <kernel/process.hpp>

#define PAGE_SIZE 0x1000

extern "C" {
// memory.s
extern void _x86_set_page_directory(u32 addr);
extern void _x86_refresh_page_directory();
}

namespace kernel {
class Process;
}

namespace kernel::x86 {
extern u32 kernel_page_table[1024] __attribute__((aligned(4096)));
extern u32 kernel_page_directory[1024] __attribute__((aligned(4096)));
constexpr u32 kernel_address_space_dir_index = 768;

void init_memory_management();

// Allocate a physical page to use in the kernel, available to all kernel tasks.
// TODO: use a pair structure thing
auto map_kernel_memory(u32 page_count) -> VirtualAddress;
auto map_user_memory(Process &process, VirtualAddress at) -> bool;
void set_user_mem_no_write(Process &process, VirtualAddress at);

} // namespace kernel::x86
