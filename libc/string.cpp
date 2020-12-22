#include "string.h"

size_t strlen(const char *str) {
  size_t len = 0;
  if (str == nullptr) {
    return 0;
  }
  while (*(str++)) {
    len++;
  }
  return len;
}

char *strcpy(char *dest, const char *src) {
  return static_cast<char *>(memcpy(dest, src, strlen(src)));
}

int strcmp(const void *lhs, const void *rhs) {
  // TODO: implement min()
  auto s1 = strlen(reinterpret_cast<const char *>(lhs));
  auto s2 = strlen(reinterpret_cast<const char *>(rhs));
  auto smallest = s1 < s2 ? s1 : s2;
  return memcmp(lhs, rhs, smallest);
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

int memcmp(const void *lhs, const void *rhs, size_t count) {
  const auto *lhs_s = reinterpret_cast<const char *>(lhs);
  const auto *rhs_s = reinterpret_cast<const char *>(rhs);
  for (size_t i = 0; i < count; i++) {
    if (lhs_s[i] < rhs_s[i])
      return -1;
    if (lhs_s[i] > rhs_s[i])
      return 1;
  }
  return 0;
}
