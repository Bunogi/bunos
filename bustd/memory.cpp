#include <bustd/memory.hpp>

namespace bustd::mem {
usize strlen(const char *str) {
  usize len = 0;
  while (*(str++)) {
    len++;
  }
  return len;
}

void *memcpy(void *dest, const void *src, usize n) {
  for (usize i = 0; i < n; i++) {
    *(static_cast<u8 *>(dest) + i) = *(static_cast<const u8 *>(src) + i);
  }
  return dest;
}
} // namespace bustd::mem
