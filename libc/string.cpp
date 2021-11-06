#include <bustd/assert.hpp>
#include <bustd/math.hpp>
#include <errno.h>
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

char *strncpy(char *dest, const char *src, size_t n) {
  size_t i = 0;
  for (i = 0; i < n && src[i] != '\0'; i++) {
    dest[i] = src[i];
  }
  for (; i < n; i++) {
    dest[i] = '\0';
  }
  return dest;
}

int strcmp(const char *lhs, const char *rhs) {
  const auto len1 = strlen(lhs);
  const auto len2 = strlen(rhs);
  if (len1 < len2) {
    return -1;
  }
  if (len1 > len2) {
    return 1;
  }
  return memcmp(lhs, rhs, len1);
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

void *memmove(void *dest_in, const void *src_in, size_t count) {
  if (count == 0) {
    return dest_in;
  }
  if (dest_in == src_in) {
    return dest_in;
  }
  const auto dest_val = reinterpret_cast<uintptr_t>(dest_in);
  const auto src_val = reinterpret_cast<uintptr_t>(src_in);
  auto *dest = reinterpret_cast<char *>(dest_in);
  const auto *src = reinterpret_cast<const char *>(src_in);

  if (dest_val < src_val && dest_val + count >= src_val) {
    // Destination before source, so we can just copy directly
    // Could have called memcpy here, but this way we are protected against
    // memcpy being implemented in a faster way, like with SIMD
    for (usize i = 0; i < count; i++) {
      *dest++ = *src++;
    }
  } else if (src_val > dest_val && src_val + count >= dest_val) {
    // Source before destination, so we can copy from the back
    auto *reverse_dest = dest + count;
    auto *reverse_src = src + count;
    for (usize i = 0; i < count; i++) {
      *reverse_dest-- = *reverse_src--;
    }
  } else {
    // No overlap detected
    memcpy(dest_in, src_in, count);
  }

  return dest_in;
}

char *strerror(int errnum) {
  static char buffer[20];
  const auto s = __bunos_libc::get_error_string(errnum);
  ASSERT(sizeof buffer > strlen(s));
  strncpy(buffer, s, sizeof buffer);
  return buffer;
}
