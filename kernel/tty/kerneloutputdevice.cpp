#include "kerneloutputdevice.hpp"

namespace kernel::tty {
KernelOutputDevice::KernelOutputDevice(x86::Vga &&vga) : m_vga(bu::move(vga)) {}

void KernelOutputDevice::write(const char *buf, usize length) {
  m_vga.write(buf, length);
  m_serial.write(buf, length);
}
} // namespace kernel::tty
