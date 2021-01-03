#pragma once

#include <bustd/math.hpp>
#include <bustd/stddef.hpp>

namespace kernel::x86::pit {
constexpr u32 chip_frequency_hz = 1193182;
constexpr u32 reload_value = 0x0400;
constexpr u32 irq_frequency_hz = chip_frequency_hz / reload_value;
constexpr u32 ticks_per_ms = irq_frequency_hz / 1000;

void initialize();

} // namespace kernel::x86::pit
