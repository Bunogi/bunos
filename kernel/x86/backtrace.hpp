#pragma once

#include <bustd/stddef.hpp>

namespace kernel::x86 {

[[gnu::always_inline]]
inline auto current_stackframe() -> usize {
  usize frame = 0;
  asm("movl %0, ebp" : "=r"(frame));
  return frame;
}

} // namespace kernel::x86
