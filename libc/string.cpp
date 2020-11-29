#include "string.h"

size_t strlen(const char *str) {
  size_t len = 0;
  while (*(str++)) {
    len++;
  }
  return len;
}

void *strcpy(char *dest, const char *src) {
  return memcpy(dest, src, strlen(src));
}

void *memcpy(void *dest, const void *src, size_t n) {
  for (size_t i = 0; i < n; i++) {
    *(static_cast<char *>(dest) + i) = *(static_cast<const char *>(src) + i);
  }
  return dest;
}

void *memset(void *ptr, int value, size_t n) {
  unsigned char *buf = static_cast<unsigned char *>(ptr);
  for (size_t i = 0; i < n; i++) {
    buf[i] = static_cast<unsigned char>(value);
  }
  return ptr;
}
