#pragma once

#include "tty/i386/vga.hpp"

namespace kernel::print {
void init(tty::Vga *dev);
} // namespace kernel::print

void kprint(const char *message);
void kprintln(const char *message);
void kprint_number(u32 v);
