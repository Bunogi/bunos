#pragma once

#include <bustd/stddef.hpp>

namespace kernel::memory::x86 {
extern u8 gdt_data;
void setup_gdt();
} // namespace kernel::memory::x86
