#pragma once

#include <bustd/stringview.hpp>
#include <kernel/tty/ittydevice.hpp>

namespace kernel::print {
void init(kernel::tty::IDevice *device);
void flush();
void write(bu::StringView chars);
} // namespace kernel::print
