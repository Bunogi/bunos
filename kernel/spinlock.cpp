#include <bustd/assert.hpp>
#include <kernel/interruptmanager.hpp>
#include <kernel/spinlock.hpp>
#include <kernel/timer.hpp>

namespace kernel {
SpinLock::SpinLock() : m_valid(true), m_locked(false) {}

SpinLock::Guard SpinLock::lock() {
  auto *instance = InterruptManager::instance();
  while (1) {
    // Make the operation atomic
    const auto guard = instance->disable_interrupts_guarded();
    ASSERT(m_valid);
    if (m_locked) {
      continue;
    }

    m_locked = true;
    return Guard(*this);
  }
}

void SpinLock::unlock() {
  const auto guard = InterruptManager::instance()->disable_interrupts_guarded();
  ASSERT(m_locked);
  m_locked = false;
}

SpinLock::Guard::Guard(SpinLock &parent) : m_parent(parent) {}
SpinLock::Guard::~Guard() { m_parent.unlock(); }

} // namespace kernel
