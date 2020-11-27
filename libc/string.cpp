#include "string.h"

size_t strlen(const char *str) {
  size_t len = 0;
  while (*(str++)) {
    len++;
  }
  return len;
}

void *memcpy(void *dest, const void *src, size_t n) {
  for (size_t i = 0; i < n; i++) {
    *(static_cast<char *>(dest) + i) = *(static_cast<const char *>(src) + i);
  }
  return dest;
}
