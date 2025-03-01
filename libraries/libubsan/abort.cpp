#include <kernel/panic.hpp>
#include <libc/stdio.h>
#include <libstacktrace/stackwalker.hpp>
#include <libubsan/abort.hpp>

namespace ubsan {
void abort(const char *violation, const char *during, const SourceLocation &loc,
           const usize address) {

  printf("\nUbsan: %s during %s object at %p at %s:%d:%d\n", violation, during,
         address, loc.filename, loc.line, loc.column);

  stacktrace::StackWalker walker(stacktrace::current_stackframe());
  walker.dump("Ubsan");
  printf("Ubsan: Aborted\n");

  kernel::panic_notrace();
}
} // namespace ubsan
