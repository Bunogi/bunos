#pragma once

#include <bustd/macros.hpp>
#include <bustd/ringbuffer.hpp>
#include <kernel/tty/ittydevice.hpp>

namespace kernel::x86::tty {
class Serial final : public kernel::tty::IDevice {
  BU_NOCOPY(Serial)
public:
  Serial();
  virtual void write(const char *buf, usize length) override;
  virtual void flush() override;

  void transmit();

private:
  bu::SizedRingBuffer<64 * 16> m_buffer;
};
} // namespace kernel::x86::tty
