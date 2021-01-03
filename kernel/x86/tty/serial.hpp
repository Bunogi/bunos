#pragma once

#include <bustd/macros.hpp>
#include <bustd/ringbuffer.hpp>
#include <kernel/tty/ittydevice.hpp>

namespace kernel::tty::x86 {
class Serial final : public IDevice {
  BU_NOCOPY(Serial)
public:
  Serial();
  virtual void write(const char *buf, usize length) override;

  void transmit();
  static Serial *instance();

private:
  bu::SizedRingBuffer<64 * 16> m_buffer;
};
} // namespace kernel::tty::x86
