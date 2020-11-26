#include "to_string.hpp"
#include "panic.hpp"

static char BUFFER[12];

static char from_nibble(u8 nibble) {
  switch (nibble) {
  case 0x0:
    return '0';
  case 0x1:
    return '1';
  case 0x2:
    return '2';
  case 0x3:
    return '3';
  case 0x4:
    return '4';
  case 0x5:
    return '5';
  case 0x6:
    return '6';
  case 0x7:
    return '7';
  case 0x8:
    return '8';
  case 0x9:
    return '9';
  case 0xA:
    return 'A';
  case 0xB:
    return 'B';
  case 0xC:
    return 'C';
  case 0xD:
    return 'D';
  case 0xE:
    return 'E';
  case 0xF:
    return 'F';
  default:
    ASSERT_NOT_REACHED();
    return 0;
  }
}

namespace kernel {

char *to_string_hex(u32 v) {
  BUFFER[0] = '0';
  BUFFER[1] = 'x';

  for (int i = 0; i < 8; i++) {
    const u8 shift_amount = (7 - i) * 4;
    const u8 this_byte = (v >> shift_amount) & 0xF;
    BUFFER[i + 2] = from_nibble(this_byte);
  }
  BUFFER[2 + 8 + 1] = '\0';
  return BUFFER;
}
} // namespace kernel
