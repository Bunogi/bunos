#pragma once

#include <kernel/x86/tty/serial.hpp>
#include <kernel/x86/tty/vga.hpp>

namespace kernel::tty {
// Default
class KernelOutputDevice : public IDevice {
public:
  virtual void write(const char *buf, usize length) override;
  virtual void putchar(const char c) override;

private:
  x86::Serial m_serial;
  x86::Vga m_vga;
};
} // namespace kernel::tty
