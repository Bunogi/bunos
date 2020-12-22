#pragma once

#include <bustd/stddef.hpp>

namespace kernel::memory::x86 {
constexpr u64 to_physical_address(u64 address);
void init_memory_management();
} // namespace kernel::memory::x86
