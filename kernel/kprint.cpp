#include "kprint.hpp"
#include "panic.hpp"
#include "to_string.hpp"
#include "tty/i386/vga.hpp"

#include <bustd/assert.hpp>

static kernel::tty::Vga *out_device;

namespace kernel::print {
void init(tty::Vga *dev) { out_device = dev; }
} // namespace kernel::print

void kprint(const char *message) {
  ASSERT_TRUE(message != nullptr);
  ASSERT_TRUE(out_device != nullptr);
  out_device->print(message);
}

void kprintln(const char *message) {
  kprint(message);
  out_device->putchar('\n');
}

void kprint_number(u32 v) { out_device->print(kernel::to_string_hex(v)); }
