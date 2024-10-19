#pragma once

#include <bustd/macros.hpp>
#include <kernel/x86/interrupts.hpp>

namespace kernel::interrupts {
void init();

auto register_handler(u16 vector, x86::InterruptHandler handler) -> u16;
void unregister_handler(usize handler_id); // maybe unneeded?

auto handle_interrupt(x86::InterruptFrame *frame) -> bool;

void disable_non_printing_interrupts();

[[gnu::always_inline]] inline auto enabled() -> bool {
  return x86::interrupts_enabled();
}

auto is_in_isr() -> bool;

} // namespace kernel::interrupts
