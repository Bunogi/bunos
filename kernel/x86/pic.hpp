#pragma once

#include <bustd/stddef.hpp>

namespace kernel::interrupt::x86::pic {
void initialize();
static constexpr u8 master_vector_offset = 0x20;
static constexpr u8 slave_vector_offset = 0x28;
void unmask_irq(u8 irq);
void acknowledge(u8 irq);
bool check_spurious(u8 irq);
} // namespace kernel::interrupt::x86::pic
