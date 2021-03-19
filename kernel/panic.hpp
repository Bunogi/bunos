#pragma once

#include <bustd/stringview.hpp>
#include <kernel/x86/interrupts.hpp>

#define KERNEL_PANIC(_r) ::kernel::panic_in_code(__FILE__, __LINE__, _r);

namespace kernel {
[[noreturn]] void panic_in_code(const char *const file, const u32 line,
                                const bu::StringView reason);
[[noreturn]] void panic_from_interrupt(x86::InterruptFrame *frame,
                                       const bu::StringView reason,
                                       bool has_errcode);
bool in_panic();
} // namespace kernel
