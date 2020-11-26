#include "panic.hpp"
#include "kprint.hpp"

namespace kernel::panic {
void panic() {
  print::print("KERNEL PANIC");
  volatile bool run = 1;
  while (run) {
  }
}
} // namespace kernel::panic
