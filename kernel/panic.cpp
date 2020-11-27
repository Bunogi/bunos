#include "panic.hpp"
#include "kprint.hpp"

void kpanic() {
  u32 eax, ebx, ecx, edx;
  __asm__("nop" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx));
  kprintln("KERNEL PANIC");

  kprint("eax: ");
  kprint_number(eax);
  kprint(", ebx: ");
  kprint_number(ebx);
  kprint(", ecx: ");
  kprint_number(ecx);
  kprint(", edx: ");
  kprint_number(edx);
  kprintln("");

  volatile bool run = 1;
  while (run) {
  }
}
