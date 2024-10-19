#include <kernel/interruptguard.hpp>
#include <kernel/x86/interrupts.hpp>

namespace kernel {
InterruptGuard::InterruptGuard() {
  if (x86::interrupts_enabled()) {
    x86::cli();
    m_did_disable_interrupts = true;
  }
}

InterruptGuard::~InterruptGuard() {
  if (m_did_disable_interrupts) {
    x86::sti();
  }
}

auto InterruptGuard::disabled_interrupts() const -> bool {
  return m_did_disable_interrupts;
}

} // namespace kernel
