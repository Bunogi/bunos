#pragma once

#include <bustd/macros.hpp>
#include <kernel/x86/interrupts.hpp>

namespace kernel::interrupts {
void init();

u16 register_handler(u16 vector, x86::InterruptHandler handler);
void unregister_handler(usize handler_id); // maybe unneeded?

bool handle_interrupt(x86::InterruptFrame *frame);

void disable_non_printing_interrupts();

[[gnu::always_inline]] inline bool enabled() {
  return x86::interrupts_enabled();
}

bool is_in_isr();

} // namespace kernel::interrupts
