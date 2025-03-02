#include <bustd/assert.hpp>
#include <libc/stdio.h>
#include <libstacktrace/stackwalker.hpp>

#ifdef __IN_KERNEL__
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

auto StackWalker::current_function() -> const char * {
  const char *retval = "<unknown>";

  if (m_current_frame == nullptr) {
    return retval;
  }
#ifdef __IN_KERNEL__
  retval = kernel::function_name_from_pc(m_current_frame->instruction_pointer);
#endif
  return retval;
}

auto StackWalker::dump(const char *prefix) -> void {
  while (m_current_frame) {
    if (prefix) {
      printf("%s: ", prefix);
    }

    printf("%p %s\n", m_current_frame->instruction_pointer, current_function());

    up();
  }
}

} // namespace stacktrace
