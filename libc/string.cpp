#include <bustd/math.hpp>
#include <string.h>

size_t strlen(const char *str) {
  size_t len = 0;
  while (*(str++)) {
    len++;
  }
  return len;
}

char *strcpy(char *dest, const char *src) {
  return static_cast<char *>(memcpy(dest, src, strlen(src)));
}

int strcmp(const char *lhs, const char *rhs) {
  auto s1 = strlen(lhs);
  auto s2 = strlen(rhs);
  return memcmp(lhs, rhs, bu::min(s1, s2));
}

int strncmp(const char *lhs, const char *rhs, size_t n) {
  for (size_t i = 0; i < n; i++) {
    if (*lhs == '\0' && *rhs == '\0') {
      return 0;
    } else if (*lhs == '\0') {
      return -1;
    } else if (*rhs == '\0') {
      return 1;
    }

    if (*lhs < *rhs) {
      return -1;
    } else if (*lhs > *rhs) {
      return 1;
    }
  }
  return 0;
}

void *memcpy(void *dest, const void *src, size_t n) {
  for (size_t i = 0; i < n; i++) {
    *(reinterpret_cast<char *>(dest) + i) =
        *(reinterpret_cast<const char *>(src) + i);
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
