#include <kernel/kprint.hpp>
#include <stdio.h>

namespace kernel::print {
kernel::tty::IDevice *out_device;

void init(tty::IDevice *dev) {
  out_device = dev;
  printf("Successfully initted tty at 0x%.8X\n", dev);
}

void flush() { out_device->flush(); }

void write(bu::StringView s) { out_device->write(s.data(), s.len()); }
} // namespace kernel::print
