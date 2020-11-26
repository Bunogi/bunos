#pragma once

#include <bustd/stddef.hpp>

namespace kernel {
// TODO: replace with something dynamic
// NOTE: these all use the same underlying buffer
char *to_string_hex(u32 v);
} // namespace kernel
