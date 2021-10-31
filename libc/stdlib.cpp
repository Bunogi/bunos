#include <bustd/assert.hpp>
#include <bustd/math.hpp>
#include <bustd/stddef.hpp>
#include <ctype.h>
#include <kernel/syscalls.hpp>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __IN_KERNEL__
#include <kernel/kmalloc.hpp>
#define NON_KERNELSPACE(name)                                                  \
  KERNEL_PANIC(name "is not defined in kernel-space!")
#endif

int atexit(void (*function)(void)) {
  (void)function;
#ifdef __IN_KERNEL__
  NON_KERNELSPACE("atexit");
#else
  TODO();
#endif
  return 0;
}

char *getenv(const char *) {
#ifdef __IN_KERNEL__
  NON_KERNELSPACE("getenv");
#else
  TODO();
#endif
  return nullptr;
}

// It complains that we leak here, but that's the idea!
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wanalyzer-malloc-leak"
void *malloc(size_t size) {
  if (size == 0) {
    return nullptr;
  }
#ifdef __IN_KERNEL__
  return kernel::malloc::Allocator::instance()->allocate(size);
#else
  (void)size;
  TODO();
#endif
}

void *calloc(size_t nmemb, size_t size) {
  const auto block_size = nmemb * size;
  if (block_size == 0) {
    return nullptr;
  }

  auto *out = malloc(block_size);
  memset(out, 0, block_size);
  return out;
}
#pragma GCC diagnostic pop

void free(void *ptr) {
  if (ptr == nullptr) {
    return;
  }
#ifdef __IN_KERNEL__
  return kernel::malloc::Allocator::instance()->deallocate(ptr);
#else
  (void)ptr;
  TODO();
#endif
}

int atoi(const char *ptr) { return strtol(ptr, nullptr, 10); }

long atol(const char *ptr) { return strtol(ptr, nullptr, 10); }

long long atoll(const char *ptr) { return strtoll(ptr, nullptr, 10); }

long long strtoll(const char *ptr, char **endptr, int base) {
  if ((base != 0 && base < 2) || base >= 36) {
    if (endptr) {
      *endptr = const_cast<char *>(ptr);
    }
    return 0;
  }

  auto *current_ptr = ptr;
  while (isspace(*current_ptr)) {
    current_ptr++;
  }

  bool negative = *current_ptr == '-';
  if (negative || *current_ptr == '+') {
    current_ptr++;
  }

  // Autodetect base if necesarry
  const auto len = strlen(current_ptr);
  if (base == 0) {
    if (len > 2 && strncmp(current_ptr, "0x", 2) == 0) {
      base = 16;
      current_ptr += 2;
    } else if (len >= 1 && *current_ptr == '0') {
      base = 8;
    }
  }

  // FIXME: Do we have to do this earlier?
  auto *end = current_ptr;
  while (*end) {
    if (isalpha(*end)) {
      if (toupper(*end) - 'A' > base) {
        break;
      }
      end++;
    } else if (isdigit(*end)) {
      if (*end - '0' > base) {
        break;
      }
      end++;
    } else {
      break;
    }
  }

  if (endptr != nullptr && ptr == end) {
    *endptr = const_cast<char *>(ptr);
    return 0;
  }

  const auto digits = static_cast<uintptr_t>(end - current_ptr) - 1;
  long long out = 0;
  for (uintptr_t i = 0; i <= digits; i++) {
    const auto pow = bu::pow(base, static_cast<int>(i));
    const auto digit = current_ptr[digits - i];
    // printf("strtoll: %u^^%u: 0x%.8x\n", base, i, pow);
    auto to_add = pow;
    if (isdigit(digit)) {
      // printf("%c converted: %u\n", digit, digit - '0');
      to_add *= digit - '0';
    } else {
      // printf("%c converted: %u\n", digit, toupper(digit) - 'A' + 10);
      to_add *= toupper(digit) - 'A' + 10;
    }

    // printf("strtoll: digit: '%c' in position %u, to_add: %u\n", digit, i,
    //       to_add);
    ASSERT(isxdigit(digit));
    // FIXME: Handle overflows correctly
    out += to_add;
  }
  current_ptr += digits;

  if (negative) {
    out = -out;
  }

  if (endptr != nullptr) {
    *endptr = const_cast<char *>(end);
  }

  if (base == 16) {
    // printf("strtoll: '%s' == 0x%.8X\n", ptr, out);
  } else {
    // printf("strtoll: '%s' == %d\n", ptr, out);
  }

  return out;
  // FIXME: Have to set errno...
}

long strtol(const char *ptr, char **end, int base) {
  // FIXME: Errors
  return strtoll(ptr, end, base);
}

#ifndef __IN_KERNEL__
extern "C" {
extern void _fini();
}
#endif

void exit(int code) {
  syscall(SYS_EXIT, code);
  UNREACHABLE();
}

void abort() {
#ifdef __IN_KERNEL__
  KERNEL_PANIC("Abort() called!");
#else
  puts("abort() called!");
  exit(1);
#endif
  while (1) {
  }
}
