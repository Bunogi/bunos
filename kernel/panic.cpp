#include "panic.hpp"
#include <bustd/stddef.hpp>
#include <stdio.h>

void kpanic() {
  u32 eax, ebx, ecx, edx;
  __asm__("nop" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx));

  printf("====KERNEL_PANIC====\n"
         "eax: %.8X, ebx: %.8X, ecx: %.8X, edx: %.8X",
         eax, ebx, ecx, edx);

  volatile bool run = 1;
  while (run) {
  }
}
