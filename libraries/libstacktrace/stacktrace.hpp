#pragma once

#include <bustd/optional.hpp>
#include <bustd/stddef.hpp>

namespace stacktrace {

class StackWalker {

public:
  StackWalker(u32 frame_start);
  auto current_function() -> bu::Optional<const char *>;
  // TODO: Parameter to point this somewhere else
  auto dump(const char *prefix) -> void;

  static auto function_from_instruction_pointer(const void *) -> const char *;

private:
  auto up() -> void;
  // x86(maybe 64?) only
  struct RawFrame {
    RawFrame *next;            // ebp
    void *instruction_pointer; // eip
  };
  RawFrame *m_current_frame;
};

[[gnu::always_inline]]
inline auto current_stackframe() -> usize {
  usize frame = 0;
  asm("movl %0, ebp" : "=r"(frame));
  return frame;
}

} // namespace stacktrace
