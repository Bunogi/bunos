#include <bustd/assert.hpp>
#include <bustd/math.hpp>
#include <kernel/interrupts.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/spinlock.hpp>
#include <kernel/timer.hpp>
#include <kernel/x86/pit.hpp>
#include <kernel/x86/tty/serial.hpp>

// TODO: (maybe) use a static vector or something
static constexpr u32 SLEEP_TIMER_COUNT = 16;
static volatile u32 SLEEP_TIMERS[SLEEP_TIMER_COUNT];

static kernel::SpinLock s_lock;

extern kernel::Scheduler *s_scheduler;

namespace kernel::timer {

auto interrupt_handler(x86::InterruptFrame *frame) -> bool {
  for (u32 i = 0; i < SLEEP_TIMER_COUNT; i++) {
    if (SLEEP_TIMERS[i] != 0) {
      SLEEP_TIMERS[i]--;
    }
  }
  // FIXME: Allow things to register timer callbacks?
  // We need this to flush serial data to the port. Maybe a better way is to
  // have the serial port listen for timer interrupts directly, but that
  // requires a way to register multiple listeners on a single interrupt.
  auto *const instance = x86::tty::Serial::instance();
  if (instance != nullptr) {
    instance->transmit();
  }

  if (s_scheduler) {
    s_scheduler->wake(frame);
  }

  return true;
}

void initialize() {
  x86::pit::initialize();
  kernel::interrupts::register_handler(0x20, &interrupt_handler);
}

auto register_timer(usize ticks) -> u32 {
  // TODO: should be a mutex or something when this is actually used
  const auto guard = s_lock.lock();
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
  // FIXME: This should yield
  const auto index = register_timer(ms * x86::pit::ticks_per_ms);
  while (SLEEP_TIMERS[index] != 0) {
    __asm__ volatile("nop");
  }
}
} // namespace kernel::timer
