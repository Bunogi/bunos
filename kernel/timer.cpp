#include <bustd/assert.hpp>
#include <bustd/math.hpp>
#include <kernel/timer.hpp>
#include <kernel/x86/interruptmanager.hpp>
#include <kernel/x86/pit.hpp>

// TODO: (maybe) use a static vector or something
static constexpr u32 SLEEP_TIMER_COUNT = 16;
static volatile u32 SLEEP_TIMERS[SLEEP_TIMER_COUNT];

namespace kernel::timer {

bool interrupt_handler(interrupt::x86::InterruptFrame *) {
  for (u32 i = 0; i < SLEEP_TIMER_COUNT; i++) {
    if (SLEEP_TIMERS[i] != 0) {
      SLEEP_TIMERS[i]--;
    }
  }
  return true;
}

void initialize() {
  x86::pit::initialize();
  interrupt::x86::InterruptManager::instance()->register_handler(
      0x20, &interrupt_handler);
}

u32 register_timer(usize ticks) {
  // TODO: should be a mutex or something when this is actually used
  const auto guard = interrupt::x86::InterruptManager::instance()
                         ->disable_interrupts_guarded();
  for (u32 i = 0; i < SLEEP_TIMER_COUNT; i++) {
    if (SLEEP_TIMERS[i] == 0) {
      SLEEP_TIMERS[i] = bu::max(ticks, 1ul);
      return i;
    }
  }
  KERNEL_PANIC("Ran out of sleep timers");
  return 0;
}

void delay(usize ms) {
  const auto index = register_timer(ms * kernel::timer::x86::pit::ticks_per_ms);
  while (SLEEP_TIMERS[index] != 0) {
    __asm__ volatile("nop");
  }
}
} // namespace kernel::timer
