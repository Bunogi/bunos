#include <bustd/assert.hpp>
#include <kernel/interruptguard.hpp>
#include <kernel/spinlock.hpp>
#include <kernel/timer.hpp>

namespace kernel {
SpinLock::SpinLock() : m_valid(true), m_locked(false) {}

SpinLock::Guard SpinLock::lock() {
  while (1) {
    // Make the operation atomic
    const InterruptGuard guard;
    ASSERT(m_valid);
    if (m_locked) {
      continue;
    }

    m_locked = true;
    return Guard(*this);
  }
}

void SpinLock::unlock() {
  const InterruptGuard guard;
  ASSERT(m_locked);
  m_locked = false;
}

bool SpinLock::is_locked() { return m_locked; }

SpinLock::Guard::Guard(SpinLock &parent) : m_parent(parent) {}
SpinLock::Guard::~Guard() { m_parent.unlock(); }

} // namespace kernel
