#pragma once

#include <bustd/ringbuffer.hpp>
#include <kernel/spinlock.hpp>
#include <libc/sys/types.h>

namespace kernel::process {
// TODO: This needs to be part of the VFS eventually, and maybe support more
// than one reader
class KeyboardFile {
public:
  static auto instance() -> KeyboardFile &;
  // TODO: This won't be good long term but works now
  void key_trigger(char c);
  auto open(pid_t new_pid) -> bool;
  auto close(pid_t old_pid) -> bool;
  auto read(u8 *buffer, usize len) -> isize;

private:
  volatile pid_t m_owning_pid{0};
  bu::SizedRingBuffer<10> m_buffer;
  // FIXME: We want to use something better than a spinlock here
  SpinLock m_lock;

  KeyboardFile();
  KeyboardFile(const KeyboardFile &) = delete;
  KeyboardFile(const KeyboardFile &&) = delete;
  auto operator=(const KeyboardFile &) -> KeyboardFile & = delete;
  auto operator=(const KeyboardFile &&) -> KeyboardFile & = delete;
};
} // namespace kernel::process
