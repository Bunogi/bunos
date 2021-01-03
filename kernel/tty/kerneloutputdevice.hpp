#pragma once

#include <bustd/macros.hpp>
#include <kernel/x86/tty/serial.hpp>
#include <kernel/x86/tty/vga.hpp>

namespace kernel::tty {
// Default
class KernelOutputDevice final : public IDevice {
  BU_NOCOPY(KernelOutputDevice)
public:
  KernelOutputDevice(x86::tty::Vga &&vga);
  virtual void write(const char *buf, usize length) override;
  virtual void flush() override;

private:
  x86::tty::Serial m_serial;
  x86::tty::Vga m_vga;
};
} // namespace kernel::tty
