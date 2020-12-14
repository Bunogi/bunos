#pragma once

#include "x86/interrupts.hpp"
#include <bustd/string_view.hpp>

#define KERNEL_PANIC(_r) ::kernel::panic_in_code(__FILE__, __LINE__, _r);

namespace kernel {
void panic_in_code(const char *const file, const u32 line,
                   const bustd::StringView reason);
void panic_from_interrupt(interrupt::x86::InterruptFrame *frame,
                          const bustd::StringView reason, bool has_errcode);
} // namespace kernel
