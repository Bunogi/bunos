#pragma once

#include <bustd/stddef.hpp>

namespace kernel::x86 {
void set_io_permissions();

void out_u8(u16 port, u8 out);
// TODO: Maybe implement these other ones later
// void out_u16(u16 port, u16 out);
// void out_u32(u16 port, u32 out);

// Wrapper around asm routine
void out_u8_string(u16 port, const u8 *buffer, usize length);

auto in_u8(u16 port) -> u8;
auto in_u16(u16 port) -> u16;
// u32 in_u32(u16 port);
// void in_string(/*something*/)
void in_u16_string(u16 port, u16 *buffer, usize length);

} // namespace kernel::x86
