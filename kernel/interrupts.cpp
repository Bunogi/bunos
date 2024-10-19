#include <bustd/assert.hpp>
#include <kernel/interrupts.hpp>
#include <kernel/x86/pic.hpp>
#include <string.h>

namespace {
volatile bool is_in_isr{false};
}

namespace kernel::interrupts {
// TODO: Allow multiple listeners on the same interrupt?
x86::InterruptHandler s_handlers[256]{};

void init() {
  x86::initialize_interrupts();
  x86::pic::initialize();
}

auto register_handler(u16 vector, x86::InterruptHandler handler) -> u16 {
  constexpr u8 usable_vector_offset = 32;
  // Reserved for exceptions
  ASSERT(vector >= usable_vector_offset);
  // Currently we have no way to add multiple listeners(TODO: Should we?)
  ASSERT_EQ(s_handlers[vector], nullptr);

  s_handlers[vector] = handler;

  // FROM PIC
  if (vector >= usable_vector_offset &&
      vector < x86::pic::slave_vector_offset + 8) {
    x86::pic::unmask_irq(vector - usable_vector_offset);
  }

  return vector;
}

auto handle_interrupt(x86::InterruptFrame *frame) -> bool {
  if (frame->int_vector == 0x27 || frame->int_vector == 0x2f) {
    return true;
  }
  ::is_in_isr = true;
  if (s_handlers[frame->int_vector] != nullptr) {
    const auto retval = s_handlers[frame->int_vector](frame);
    if (retval && frame->int_vector < 0x30) {
      x86::pic::acknowledge(frame->int_vector - 0x20);
    }
    ::is_in_isr = false;
    return retval;
  } else {
    ::is_in_isr = false;
    return false;
  }
}

void disable_non_printing_interrupts() { x86::pic::mask_non_printing_irqs(); }

auto is_in_isr() -> bool { return ::is_in_isr; }
} // namespace kernel::interrupts
