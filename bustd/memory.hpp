#pragma once

#include <bustd/stddef.hpp>

namespace bustd::mem {
usize strlen(const char *str);
void *memcpy(void *dest, const void *src, usize n);
} // namespace bustd::mem
