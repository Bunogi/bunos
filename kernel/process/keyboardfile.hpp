#pragma once

#include <bustd/ringbuffer.hpp>
#include <kernel/spinlock.hpp>
#include <libc/sys/types.h>

namespace kernel::process {
// TODO: This needs to be part of the VFS eventually, and maybe support more
// than one reader
class KeyboardFile {
public:
  static KeyboardFile &instance();
  // TODO: This won't be good long term but works now
  void key_trigger(char c);
  bool open(pid_t new_pid);
  bool close(pid_t old_pid);
  isize read(u8 *buffer, usize len);

private:
  volatile pid_t m_owning_pid{0};
  bu::SizedRingBuffer<10> m_buffer;
  // FIXME: We want to use something better than a spinlock here
  SpinLock m_lock;

  KeyboardFile();
  KeyboardFile(const KeyboardFile &) = delete;
  KeyboardFile(const KeyboardFile &&) = delete;
  KeyboardFile &operator=(const KeyboardFile &) = delete;
  KeyboardFile &operator=(const KeyboardFile &&) = delete;
};
} // namespace kernel::process
