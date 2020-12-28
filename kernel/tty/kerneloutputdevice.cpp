#include "kerneloutputdevice.hpp"

namespace kernel::tty {
KernelOutputDevice::KernelOutputDevice(x86::Vga &&vga) : m_vga(bu::move(vga)) {}
void KernelOutputDevice::putchar(const char c) {
  m_vga.putchar(c);
  m_serial.putchar(c);
}

void KernelOutputDevice::write(const char *buf, usize length) {
  m_vga.write(buf, length);
  m_serial.write(buf, length);
}
} // namespace kernel::tty
