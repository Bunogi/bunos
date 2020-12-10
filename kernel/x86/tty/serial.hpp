#pragma once

#include <kernel/tty/ittydevice.hpp>

namespace kernel::tty::x86 {
class Serial : public IDevice {
public:
  Serial();
  virtual void write(const char *buf, usize length) override;
  virtual void putchar(const char c) override;

private:
};
} // namespace kernel::tty::x86
