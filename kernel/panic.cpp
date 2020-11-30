#include "panic.hpp"
#include <bustd/stddef.hpp>
#include <stdio.h>

namespace Local {
struct StackFrame {
  StackFrame *ebp;
  uint32_t eip;
};

void print_stack_trace(StackFrame *frame) {
  printf("Stack trace:\n");
  // assumption: bottom-most stack frame has a base pointer of 0
  while (frame) {
    printf("%p\n", frame->eip);
    frame = frame->ebp;
  }
}
} // namespace Local

void kpanic() {
  u32 eax, ebx, ecx, edx;
  __asm__("nop" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx));

  printf("====KERNEL_PANIC====\n"
         "eax: %.8X, ebx: %.8X, ecx: %.8X, edx: %.8X\n",
         eax, ebx, ecx, edx);

  Local::StackFrame *frame;
  __asm__("mov %%ebp, %%eax" : "=a"(frame)::);
  Local::print_stack_trace(frame);

  volatile bool run = 1;
  while (run) {
  }
}
