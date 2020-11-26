#pragma once

#include "tty/i386/vga.hpp"

namespace kernel::print {
void init(tty::Vga *dev);
void print(const char *message);
void println(const char *message);
void number(u32 v);
} // namespace kernel::print
