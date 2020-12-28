#include <bustd/math.hpp>
#include <kernel/x86/io.hpp>
#include <kernel/x86/pit.hpp>

#include <stdio.h>

namespace kernel::timer::x86::pit {

// We don't care about channel 1 because it's unusable,
// and we definitely don't care about the pc speaker(channel 2)
constexpr u16 channel_0_port = 0x40;
constexpr u16 command_register = 0x43;

void initialize() {
  // Use mode 2. This means we get interrupts half as often, and have to
  static_assert(reload_value != 1);
  static_assert(reload_value != 0);

  // Channel 0, lobyte/hibyte, mode 3(square wave)
  constexpr u8 channel = 0x00;               // bits 6 and 7, channel 0
  constexpr u8 access_mode = 0x30;           // lowbyte/hibyte
  constexpr u8 operating_mode = 0x04 | 0x02; //| 0x02; //| 0x02; // Mode 3
  kernel::x86::io::out_u8(command_register,
                          channel | access_mode | operating_mode);

  // Low byte
  kernel::x86::io::out_u8(channel_0_port, reload_value & 0xFF);
  // High byte
  kernel::x86::io::out_u8(channel_0_port, (reload_value >> 8) & 0xFF);

  printf("PIT: mode 2, reload: 0x%.4X, irq_freqency: %u ticks per ms: %u\n",
         reload_value, irq_frequency_hz, static_cast<u32>(ticks_per_ms));
}
} // namespace kernel::timer::x86::pit
