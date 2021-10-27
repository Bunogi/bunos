#include <bustd/stringview.hpp>
#include <kernel/interrupts.hpp>
#include <kernel/tty/kerneloutputdevice.hpp>

namespace kernel::tty {
KernelOutputDevice::KernelOutputDevice(x86::tty::Vga &&vga)
    : m_vga(bu::move(vga)) {}

void KernelOutputDevice::write(const char *buf, const usize length) {
  if (s_m_lock.is_locked() && interrupts::is_in_isr()) {
    // Backup solution. If this rolls over, we lose data.
    m_interrupt_buffer.write(reinterpret_cast<const u8 *>(buf), length);
    return;
  }

  const auto guard = s_m_lock.lock();
  if (!m_interrupt_buffer.is_empty()) {
    // flush interrupt buffer
    bu::StringView s1, s2;
    ASSERT(m_interrupt_buffer.read_nocopy(s1, s2));

    m_vga.write(s1.data(), s1.len());
    m_vga.write(s2.data(), s2.len());
    m_serial.write(s1.data(), s1.len());
    m_serial.write(s2.data(), s2.len());
    m_interrupt_buffer.clear();
  }

  m_vga.write(buf, length);
  m_serial.write(buf, length);
}

void KernelOutputDevice::flush() {
  m_vga.flush();
  m_serial.flush();
}

SpinLock KernelOutputDevice::s_m_lock;
} // namespace kernel::tty
