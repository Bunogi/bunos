#include <bustd/assert.hpp>
#include <bustd/optional.hpp>
#include <bustd/stringview.hpp>
#include <libc/stdio.h>
#include <libstacktrace/stacktrace.hpp>

#ifdef IN_KERNEL
#include <kernel/debugsymbols.hpp>
#endif

namespace stacktrace {
StackWalker::StackWalker(u32 frame_start) {
  ASSERT(frame_start != 0);
  m_current_frame = reinterpret_cast<StackWalker::RawFrame *>(frame_start);
}

auto StackWalker::up() -> void {
  if (m_current_frame) {
    m_current_frame = m_current_frame->next;
  }
}

auto StackWalker::current_function() -> bu::Optional<const char *> {
  if (m_current_frame == nullptr) {
    return bu::create_none<const char *>();
  }
#ifdef IN_KERNEL
  if (kernel::debug_symbols_loaded()) {
    return bu::create_some(function_from_instruction_pointer(
        m_current_frame->instruction_pointer));
  }
#endif
  return bu::create_some<const char *>("<missing debug symbols>");
}

auto StackWalker::function_from_instruction_pointer(
    [[maybe_unused]] const void *const ip) -> const char * {
#ifdef IN_KERNEL
  if (kernel::debug_symbols_loaded()) {
    return kernel::function_name_from_pc(ip);
  }
#endif
  return "<missing debug symbols>";
}

auto StackWalker::dump(const char *prefix) -> void {
  auto f = current_function();
  while (f) {
    if (prefix) {
      printf("%s: ", prefix);
    }

    printf("%p    %s\n", m_current_frame->instruction_pointer, *f);

    up();
    f = current_function();
  }
}

} // namespace stacktrace
