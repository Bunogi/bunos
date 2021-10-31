#include <bustd/assert.hpp>
#include <bustd/stddef.hpp>
#include <libc/errno.h>
#include <libc/stdio.h>
#include <libc/string.h>
#include <libc/sys/syscall.h>
#include <libc/unistd.h>
#include <stdarg.h>

FILE *stderr = 0;
FILE *stdin = 0;
FILE *stdout = 0;
int fflush(FILE *) {
  TODO();
  return 0;
}
int fprintf(int, const char *, ...) {
  TODO();
  return 0;
}

namespace {
bool is_digit(char c) { return c >= '0' && c <= '9'; }

size_t bytes_to_specifier(const char *buf) {
  size_t out = 0;
  while (*buf && is_digit(*buf)) {
    out++;
    buf++;
  }
  return out;
}

size_t pow(size_t x, size_t y) {
  if (y == 0) {
    return static_cast<size_t>(1);
  }
  size_t out = x;
  for (size_t i = 1; i < y; i++) {
    out *= x;
  }
  return out;
}

size_t string_to_padding(const char *s, size_t digits) {
  size_t out = 0;
  for (size_t i = 0; i < digits; i++) {
    ASSERT(is_digit(s[i]));
    const auto num = s[i] - '0';
    out += num * pow(10, digits - i - 1);
  }
  return out;
}

size_t to_string_hex(char *buffer, size_t v, unsigned padding, bool uppercase) {
  size_t buffer_index = 0, i;
  bool has_written_nonzero = false;

  const size_t nibble_count = sizeof v * 2;
  for (i = 1; i <= nibble_count; i++) {
    const char shift_amount = (nibble_count - i) * 4;
    const char this_nibble = (v >> shift_amount) & 0xF;

    if (this_nibble == 0) {
      if (has_written_nonzero) {
        buffer[buffer_index] = '0';
      }
      // padding starts counting from 1 rather than 0
      else if (padding > (nibble_count - i)) {
        buffer[buffer_index] = '0';
      } else {
        continue;
      }
    } else if (this_nibble <= 9) {
      buffer[buffer_index] = '0' + this_nibble;
      has_written_nonzero = true;
    } else {
      buffer[buffer_index] = 'A' + (this_nibble - 0xA);
      has_written_nonzero = true;
    }
    if (!uppercase && buffer[buffer_index] >= 'A') {
      buffer[buffer_index] += 0x20;
    }

    buffer_index++;
  }
  return buffer_index;
}

size_t to_string_unsigned(char *buffer, unsigned v, u32 padding) {
  size_t buffer_index = 0;
  bool has_written_nonzero = false;

  // TODO: ugly but it works, maybe improve
#ifdef __IS_X86__
  // 2^32 - 1
  const unsigned digits_in_max = 10;
#else
#error Unimplemented on this arch
#endif

  for (size_t i = 1; i <= digits_in_max; i++) {
    const unsigned powered = pow(10, digits_in_max - i);
    const char this_digit = (v / powered) % 10;

    if (this_digit == 0) {
      if (has_written_nonzero) {
        buffer[buffer_index] = '0';
      }
      // padding starts counting from 1 rather than 0
      else if (padding > (digits_in_max - i)) {
        buffer[buffer_index] = '0';
      } else {
        continue;
      }
    } else {
      buffer[buffer_index] = '0' + this_digit;
      has_written_nonzero = true;
    }

    buffer_index++;
  }
  return buffer_index;
}

size_t to_string_signed(char *buffer, int v, u32 padding) {
  if (v < 0) {
    buffer[0] = '-';
    return to_string_unsigned(buffer + 1, -v, padding) + 1;
  } else {
    return to_string_unsigned(buffer, v, padding);
  }
}

// TODO: make optional class and use that insdead when we get memory allocation
bool handle_integer(char *buffer, size_t *offset, char format, va_list *args,
                    size_t padding) {
  switch (format) {
  case 'x': {
    const auto v = va_arg(*args, unsigned);
    *offset += to_string_hex(buffer + *offset, v, padding, false);
    return true;
  }
  case 'X': {
    const auto v = va_arg(*args, unsigned);
    *offset += to_string_hex(buffer + *offset, v, padding, true);
    return true;
  }
  case 'i':
  case 'd': {
    const auto v = va_arg(*args, int);
    *offset += to_string_signed(buffer + *offset, v, padding);
    return true;
  }
  case 'u': {
    const auto v = va_arg(*args, unsigned);
    *offset += to_string_unsigned(buffer + *offset, v, padding);
    return true;
  }

  case 'p': {
    // Implementation specified so we can do whatever we want :)
    const auto v = va_arg(*args, uintptr_t);
    buffer[*offset] = '0';
    buffer[*offset + 1] = 'x';
    *offset += 2;
    *offset += to_string_hex(buffer + *offset, v, sizeof(uintptr_t) * 2, true);
    return true;
  }
  default:
    return false;
  }
}

bool handle_other(char *buffer, size_t *offset, char format, va_list *args) {
  switch (format) {
  case 's': {
    const auto arg = va_arg(*args, char *);
    const auto len = strlen(arg);
    memcpy(buffer + *offset, arg, len);
    *offset += len;
    return true;
  }
  case 'c': {
    const auto arg = va_arg(*args, int);
    buffer[*offset] = static_cast<char>(arg);
    *offset += 1;
    return true;
  }
  case '%':
    buffer[*offset] = '%';
    *offset += 1;
    return true;
  }
  return false;
}

int sprintf_impl(char *buffer, const char *format, va_list args) {
  ASSERT(format != nullptr);
  ASSERT(buffer != nullptr);
#define CHECKED_INC(_x)                                                        \
  do {                                                                         \
    _x += 1;                                                                   \
    if (*(_x) == 0) {                                                          \
      /*KERNEL_PANIC(nullptr); */                                              \
      return -1;                                                               \
    }                                                                          \
  } while (0)
  size_t offset = 0;

  while (*format) {
    if (*format != '%') {
      buffer[offset++] = *format;
      format++;
      continue;
    }
    CHECKED_INC(format);
    size_t padding = 0;
    if (*format == '.') {
      CHECKED_INC(format);
      const auto padding_len = bytes_to_specifier(format);
      padding = string_to_padding(format, padding_len);
      format += padding_len;
    } else {
      padding = 1;
    }
    if (handle_integer(buffer, &offset, *format, &args, padding) ||
        handle_other(buffer, &offset, *format, &args)) {
      format++;
    } else {
      // Probably a bug for now
      printf("[(s)printf]: Unrecognized format char: %c\n", *format);
      // UNREACHABLE();
      return -1;
    }
  }
#undef CHECKED_INC
  return offset;
}

} // namespace

#ifdef __IN_KERNEL__
#include <kernel/tty/ittydevice.hpp>
namespace kernel::print {
extern kernel::tty::IDevice *out_device;
}
#endif

int sprintf(char *str, const char *format, ...) {
  va_list args;
  va_start(args, format);
  auto retval = sprintf_impl(str, format, args);
  va_end(args);
  return retval;
}

int printf(const char *format, ...) {
  char buffer[256]; // TODO do something smarter

  va_list args;
  va_start(args, format);
  auto retval = sprintf_impl(buffer, format, args);
  va_end(args);
  if (retval < 0) {
    return retval;
  }
  ASSERT(static_cast<size_t>(retval) < sizeof buffer);

#ifdef __IN_KERNEL__
  if (kernel::print::out_device != nullptr) {
    kernel::print::out_device->write(buffer, retval);
  }
#else
  // FIXME: We should probably look at the result of this
  write(1, buffer, retval);
#endif

  return retval;
}

int puts(const char *s) {
#ifdef __IN_KERNEL__
  if (kernel::print::out_device != nullptr) {
    kernel::print::out_device->write(s, strlen(s));
    kernel::print::out_device->write("\n", 1);
  }

  return 0;
#else
  const auto result = syscall(SYS_WRITE, 1, s, strlen(s));
  if (result < 0) {
    return EOF;
  }

  if (syscall(SYS_WRITE, 1, "\n", 1) < 0) {
    return EOF;
  } else {
    return 1;
  }
#endif
}

void perror(const char *const s) {
  const auto err_string = __bunos_libc::get_error_string(errno);
  printf("%s: %s\n", s, err_string);
}
