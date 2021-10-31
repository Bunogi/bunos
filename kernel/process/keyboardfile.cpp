#include <bustd/ownedptr.hpp>
#include <kernel/interruptguard.hpp>
#include <kernel/process/keyboardfile.hpp>

namespace kernel::process {
KeyboardFile &KeyboardFile::instance() {
  static bu::OwnedPtr<KeyboardFile> this_(nullptr);
  if (!this_) {
    this_ = bu::OwnedPtr(new KeyboardFile());
  }
  return *this_;
}

void KeyboardFile::key_trigger(const char c) { m_buffer.push(c); }

#include <stdio.h>
bool KeyboardFile::open(const pid_t new_pid) {
  const auto guard = m_lock.lock();
  if (m_owning_pid != 0) {
    return false;
  }

  m_owning_pid = new_pid;
  const InterruptGuard int_guard;
  m_buffer.clear();
  return true;
}

bool KeyboardFile::close(const pid_t old_pid) {
  const auto guard = m_lock.lock();
  if (m_owning_pid != old_pid) {
    return false;
  }

  const InterruptGuard int_guard;
  m_owning_pid = 0;
  m_buffer.clear();
  return true;
}

isize KeyboardFile::read(u8 *buffer, usize len) {
  const auto guard = m_lock.lock();
  const InterruptGuard int_guard;
  return m_buffer.take(buffer, len);
}

KeyboardFile::KeyboardFile() {}
} // namespace kernel::process
