#pragma once

#include <kernel/tty/ittydevice.hpp>

namespace kernel::print {
void init(kernel::tty::IDevice *device);
void flush();
} // namespace kernel::print
