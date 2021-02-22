#pragma once

#include <bustd/macros.hpp>
#include <bustd/ringbuffer.hpp>
#include <kernel/x86/ps2.hpp>
#include <kernel/x86/ps2_keyboard.hpp>

namespace kernel::x86 {
class PS2Keyboard {
  BU_NOCOPY(PS2Keyboard)

public:
  PS2Keyboard(PS2Device dev, PS2DeviceType type);
  void handle_message(u8 message);

private:
  void send_command();
  void feed_scancode(u8 byte);
  u8 scancode_to_ascii(u8 code);
  u8 with_shift(u8 code);
  enum class State { ExpectingAck, ExpectingKeyCode, Relase };
  u8 m_scancode_set;
  PS2Device m_device;
  PS2DeviceType m_type;
  u8 m_shift_count{0};
  State m_state;
  bu::SizedRingBuffer<8> m_command_queue;
};
} // namespace kernel::x86
