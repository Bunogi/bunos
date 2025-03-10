#include <bustd/stddef.hpp>
#include <kernel/debugsymbols.hpp>
#include <kernel/kprint.hpp>
#include <kernel/panic.hpp>
#include <kernel/x86/interrupts.hpp>
#include <libstacktrace/stackwalker.hpp>
#include <stdio.h>

namespace {

auto exception_vector_to_string(u16 vector) -> bu::StringView {
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

static bool s_panicking;

// easier to debug
void nested_panic_check_fail() {
  kernel::print::flush();
  __asm__ volatile("cli\nhlt");
}

void nested_panic_check() {
  if (s_panicking) {
    nested_panic_check_fail();
  }
}
} // namespace

namespace kernel {
auto in_panic() -> bool { return s_panicking; }

void panic_from_interrupt(const x86::InterruptFrame *frame,
                          const char *const reason, bool has_errcode) {

  printf("====KERNEL_PANIC====\n%s\n", reason ? reason : "No message given");

  const auto exception_name = exception_vector_to_string(frame->int_vector);
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

  // It's safe to wait with this check for a while because the most likely way
  // for us to crash here is when printing the stack trace.
  nested_panic_check();
  s_panicking = true;

  printf("Panic at: %p %s\n", frame->eip,
         kernel::function_name_from_pc(frame->eip));

  printf("Stack trace:\n");
  stacktrace::StackWalker walker(frame->ebp);
  walker.dump("panic");

  panic_notrace();
}

void panic_in_code(const char *file, const u32 line, const char *const reason) {
  __asm__ volatile("cli");

  printf("====KERNEL_PANIC====\n%s", reason ? reason : "No message given");
  printf("\nLOCATION: %s:%u\n", file, line);

  // It's safe to wait with this check for a while because the most likely way
  // for us to crash here is when printing the stack trace.
  nested_panic_check();
  s_panicking = true;

  u32 frame = 0;
  asm("movl %%ebp, %0" : "=r"(frame));

  printf("Stack trace:\n");
  stacktrace::StackWalker walker(frame);
  walker.dump("panic");
  panic_notrace();
}

void panic_notrace() {
  kernel::print::flush();
  __asm__ volatile("cli\nhlt");
  while (1) {
  }
}
} // namespace kernel
