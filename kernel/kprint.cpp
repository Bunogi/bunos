#include "kprint.hpp"

#include <stdio.h>

namespace kernel::print {
kernel::tty::IDevice *out_device;

void init(tty::IDevice *dev) {
  out_device = dev;
  printf("Successfully initted tty at 0x%.8X\n", dev);
}
} // namespace kernel::print
