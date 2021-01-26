#include <kernel/tty/kerneloutputdevice.hpp>

namespace kernel::tty {
KernelOutputDevice::KernelOutputDevice(x86::tty::Vga &&vga)
    : m_vga(bu::move(vga)) {}

void KernelOutputDevice::write(const char *buf, const usize length) {
  const auto guard = s_m_lock.lock();
  m_vga.write(buf, length);
  m_serial.write(buf, length);
}

void KernelOutputDevice::flush() {
  m_vga.flush();
  m_serial.flush();
}

SpinLock KernelOutputDevice::s_m_lock;
} // namespace kernel::tty
