#pragma once

#include "tty/ittydevice.hpp"

namespace kernel::print {
void init(kernel::tty::IDevice *device);
} // namespace kernel::print
