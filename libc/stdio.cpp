#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <bustd/assert.hpp>
#include <bustd/stddef.hpp>

namespace Local {
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
    // printf("GOt 1\n");
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
    ASSERT_TRUE(is_digit(s[i]));
    const auto num = s[i] - '0';
    out += num * pow(10, digits - i - 1);
  }
  return out;
}

size_t to_string_hex(char *buffer, u32 v, u32 padding, bool uppercase) {
  u32 rest = v;
  size_t buffer_index = 0, i;
  bool has_written_nonzero = false;
  if (v == 0 && padding == 0) {
    padding = 1;
  }
  for (i = 0; i < 8; i++) {
    const char shift_amount = (7 - i) * 4;
    const char this_nibble = (v >> shift_amount) & 0xF;

    if (this_nibble == 0) {
      if (has_written_nonzero) {
        buffer[buffer_index] = '0';
      }
      // padding starts counting from 1 rather than 0
      else if (padding > (7 - i)) {
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

    // Done early
    if (rest == 0) {
      return i + 1;
    }
    rest -= (v >> shift_amount) & 0xF;
    buffer_index++;
  }
  return i;
}
} // namespace Local

#ifdef __IN_KERNEL__
#include <kernel/tty/ittydevice.hpp>
namespace kernel::print {
extern kernel::tty::IDevice *out_device;
}
#endif

int printf(const char *format, ...) {
  ASSERT_TRUE(format != nullptr);
  char buffer[256]; // TODO do something smarter

#define CHECKED_INC(_x)                                                        \
  do {                                                                         \
    _x += 1;                                                                   \
    if (*(_x) == 0) {                                                          \
      kpanic();                                                                \
      return -1;                                                               \
    }                                                                          \
  } while (0)
  size_t offset = 0;

  va_list args;
  va_start(args, 0);
  while (*format) {
    if (*format != '%') {
      buffer[offset++] = *format;
      format++;
      continue;
    }
    // TODO: Implement non-hex stuff too
    // TODO: Maybe validate the output?
    CHECKED_INC(format);
    size_t padding = 0;
    if (*format == '.') {
      CHECKED_INC(format);
      const auto padding_len = Local::bytes_to_specifier(format);
      padding = Local::string_to_padding(format, padding_len);
      format += padding_len;
    }
    size_t delta = 0;
    switch (*format) {
    case 'x': {
      const auto v = va_arg(args, unsigned);
      delta = Local::to_string_hex(buffer + offset, v, padding, false);
      break;
    }
    case 'X': {
      const auto v = va_arg(args, unsigned);
      delta = Local::to_string_hex(buffer + offset, v, padding, true);
      break;
    }
    default:
      ASSERT_NOT_REACHED(); // not implemented
    }
    format++;
    offset += delta;
  }
#undef CHECKED_INC
  va_end(args);
  ASSERT_TRUE(offset + 1 < sizeof buffer);
  buffer[offset + 1] = '\0';

#ifdef __IN_KERNEL__
  kernel::print::out_device->write(buffer, offset);
#else
#warning Printf not implemented yet :)
#endif

  return offset;
}
