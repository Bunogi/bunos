#pragma once

#include <bustd/stddef.hpp>

namespace kernel::x86 {
void init_ps2_controller();
enum class PS2DeviceType {
  MousePlain,
  MouseScrollWheel,
  MouseFiveButton,
  Keyboard,
  KeyboardWithTranslation
};

enum class PS2Device { First, Second };

namespace ps2 {
void write(const PS2Device &dev, u8 byte);
u8 read_isr_response();
} // namespace ps2
} // namespace kernel::x86
