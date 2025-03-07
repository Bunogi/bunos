#pragma once

#include <bustd/stddef.hpp>

namespace stacktrace {

class StackWalker {

public:
  StackWalker(u32 frame_start);
  auto current_function() -> const char *;
  // TODO: Parameter to point this somewhere else
  auto dump(const char *prefix) -> void;

private:
  auto up() -> void;
  // x86(maybe 64?) only
  struct RawFrame {
    RawFrame *next;            // ebp
    usize instruction_pointer; // eip
  };
  RawFrame *m_current_frame;
};

[[gnu::always_inline]]
inline auto current_stackframe() -> usize {
  usize frame = 0;
  asm("movl %%ebp, %0" : "=r"(frame));
  return frame;
}

} // namespace stacktrace
