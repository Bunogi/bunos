#pragma once

#include <bustd/ringbuffer.hpp>
#include <kernel/tty/ittydevice.hpp>

namespace kernel::tty::x86 {
class Serial : public IDevice {
public:
  Serial();
  virtual void write(const char *buf, usize length) override;
  virtual void putchar(const char c) override;

  void transmit();

private:
  bu::RingBuffer m_buffer;
};
} // namespace kernel::tty::x86
