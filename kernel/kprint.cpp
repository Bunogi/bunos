#include "kprint.hpp"
#include "panic.hpp"
#include "to_string.hpp"
#include "tty/i386/vga.hpp"

static kernel::tty::Vga *out_device;

namespace kernel::print {
void init(tty::Vga *dev) { out_device = dev; }
void print(const char *message) {
  ASSERT_TRUE(message != nullptr);
  ASSERT_TRUE(out_device != nullptr);
  out_device->print(message);
}
void println(const char *message) {
  print(message);
  out_device->putchar('\n');
}

void number(u32 v) { out_device->print(kernel::to_string_hex(v)); }
} // namespace kernel::print
