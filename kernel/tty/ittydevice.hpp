#pragma once

#include <bustd/stddef.hpp>

// In this design, a tty is any device which can be used as a dumb terminal

namespace kernel::tty {
class IDevice {
public:
  virtual void write(const char *buf, usize length) = 0;
  virtual void flush() = 0;
};
} // namespace kernel::tty
