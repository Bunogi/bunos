#include "panic.hpp"
#include <bustd/stddef.hpp>
#include <kernel/kprint.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/x86/interruptmanager.hpp>
#include <stdio.h>

namespace {
namespace Local {
struct StackFrame {
  StackFrame *ebp;
  uint32_t eip;
};

void print_stack_trace(u32 current_eip, StackFrame *frame) {
  printf("Stack trace:\n");
  if (current_eip != 0) {
    printf("%p\n", current_eip);
  }
  // assumption: bottom-most stack frame has a base pointer of 0
  while (frame) {
    printf("%p\n", frame->eip);
    frame = frame->ebp;
  }
}

bu::StringView exception_vector_to_string(u16 vector) {
  switch (vector) {
  case 0:
    return "divide error";
  case 1:
    return "debug exception";
  case 2:
    return "NMI interrupt";
  case 3:
    return "breakpoint";
  case 4:
    return "overflow";
  case 5:
    return "bound range exceeded";
  case 6:
    return "invalid opcode";
  case 7:
    return "no math coprocessor";
  case 8:
    return "double fault";
  case 9:
    return "coprocessor segment overrun(reserved)";
  case 10:
    return "invalid TSS";
  case 11:
    return "segment not present";
  case 12:
    return "stack segment fault";
  case 13:
    return "general protection fault";
  case 14:
    return "page fault";
  case 15:
    return "intel reserved";
  case 16:
    return "FPU floating point error";
  case 17:
    return "alignment check";
  case 18:
    return "machine check";
  case 19:
    return "SIMD floating floating point exception";
  case 20:
    return "virtualization exception";
  case 21:
    return "control protection";
  default:
    return nullptr;
  }
}
void disable_non_printing_interrupts() {
  kernel::Scheduler::disable();
  auto *instance = kernel::interrupt::x86::InterruptManager::instance();
  instance->disable_non_printing_interrupts();
  instance->enable_interrupts();
}
} // namespace Local
} // namespace

namespace kernel {
void panic_from_interrupt(interrupt::x86::InterruptFrame *frame,
                          const bu::StringView reason, bool has_errcode) {
  Local::disable_non_printing_interrupts();
  printf("====KERNEL_PANIC====\n"
         "%s\n",
         reason.data_or("No message given"));

  const auto exception_name =
      Local::exception_vector_to_string(frame->int_vector);
  if (has_errcode) {
    printf("==>Received an interrupt of vector 0x%.4X(%s) with error code: "
           "0x%.4X\n",
           frame->int_vector, exception_name.data_or("Unknown"),
           frame->err_code);
  } else {
    printf("==>Received an interrupt of vector 0x%.4X(%s) with no error code\n",
           frame->int_vector, exception_name.data_or("Unknown"));
  }
  if (frame->int_vector == 0xE) {
    u32 cr2;
    __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
    printf("Page fault trying to access 0x%.8X\n", cr2);
  }

  printf("==>Register dump:\n"
         "edi: 0x%.8X esi: 0x%.8X, ebp: 0x%.8X, dud_esp: 0x%"
         ".8X, ebx 0x%.8X, edx "
         "0x%.8X, ecx 0x%.8X, eax: 0x%.8X\n",
         frame->edi, frame->esi, frame->ebp, frame->dud_esp, frame->ebx,
         frame->ecx, frame->edx, frame->eax);
  printf("eip: 0x%.8X, cs: 0x%.8X, eflags: 0x%.8X useresp: 0x%.8X, ss: "
         "0x%.8X\n",
         frame->eip, frame->cs, frame->eflags, frame->useresp, frame->ss);

  Local::print_stack_trace(frame->eip,
                           reinterpret_cast<Local::StackFrame *>(frame->ebp));

  kernel::print::flush();
  __asm__ volatile("cli\nhlt");
}

void panic_in_code(const char *file, const u32 line,
                   const bu::StringView reason) {
  Local::disable_non_printing_interrupts();
  printf("====KERNEL_PANIC====\n%s", reason.data_or("No message given"));
  printf("\nLOCATION: %s:%u\n", file, line);

  Local::StackFrame *frame;
  __asm__ volatile("movl %%ebp, %0\n" : "=r"(frame));
  Local::print_stack_trace(0, frame);
  kernel::print::flush();
  __asm__ volatile("cli\nhlt");
}
} // namespace kernel
