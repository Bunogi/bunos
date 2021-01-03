#include "kerneloutputdevice.hpp"

namespace kernel::tty {
KernelOutputDevice::KernelOutputDevice(x86::Vga &&vga) : m_vga(bu::move(vga)) {}

void KernelOutputDevice::write(const char *buf, const usize length) {
  m_vga.write(buf, length);
  m_serial.write(buf, length);
}

void KernelOutputDevice::flush() {
  m_vga.flush();
  m_serial.flush();
}
} // namespace kernel::tty
