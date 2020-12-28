#pragma once

#include <bustd/stddef.hpp>

namespace kernel::timer {
void initialize();

void delay(usize ms);
} // namespace kernel::timer
