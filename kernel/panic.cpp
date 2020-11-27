#include "panic.hpp"
#include "kprint.hpp"

namespace kernel::panic {
void panic() {
  u32 eax, ebx, ecx, edx;
  __asm__("nop" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx));
  print::println("KERNEL PANIC");

  print::print("eax: ");
  print::number(eax);
  print::print(", ebx: ");
  print::number(ebx);
  print::print(", ecx: ");
  print::number(ecx);
  print::print(", edx: ");
  print::number(edx);
  print::println("");

  volatile bool run = 1;
  while (run) {
  }
}
} // namespace kernel::panic
