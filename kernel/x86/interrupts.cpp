#include <bustd/stddef.hpp>
#include <kernel/interrupts.hpp>
#include <kernel/panic.hpp>
#include <kernel/x86/handlers.inc>
#include <kernel/x86/interrupts.hpp>
#include <stdio.h>
#include <string.h>

extern "C" {
// Stuff from interrupts.S
extern void load_idt_table(u8 *address, u16 length);

#define error(_n) extern void _int_handler_vec##_n();
#define noerror(_n) error(_n)
HANDLERS(error, noerror)
#undef error
#undef noerror

// Interrupt-handler callable functions
//
void _isr_callable_error_code(kernel::x86::InterruptFrame *frame) {
  kernel::panic_from_interrupt(frame, nullptr, true);
}

void _isr_callable_noerror(kernel::x86::InterruptFrame *frame) {
  if (!kernel::interrupts::handle_interrupt(frame)) {
    kernel::panic_from_interrupt(frame, "Unhandled interrupt", false);
  }
}
}

namespace {
enum class GateType386 : u8 { Task = 0x5, Interrupt = 0xE, Trap = 0xF };

// Interrupt descriptor table data
constexpr u16 idt_entries = 256;
// Pre-allocate space for all entries
// TODO: consider aligning this by 8 for cache things as per the intel manual
// vol 3A 6-10
static u8 idt_data[idt_entries * 8];

// TODO: Privilege levels
void write_gate(u8 *buffer, void (*offset)(), GateType386 type) {

  const auto offset_constant = reinterpret_cast<uintptr_t>(offset);

  // hi offset bits
  buffer[7] = (offset_constant >> 24) & 0xFF;
  buffer[6] = (offset_constant >> 16) & 0xFF;

  buffer[5] = 0x80; // Present, ring0 only
  buffer[5] |= static_cast<u8>(type);
  buffer[4] = 0x00; // Always 0

  // Segment selector: 1
  buffer[3] = 0x00;
  buffer[2] = 0x08;

  // low offset bits
  buffer[1] = (offset_constant >> 8) & 0xFF;
  buffer[0] = offset_constant & 0xFF;
}

void setup_interrupt_handlers() {
  // TODO: Find a better way to do this
  u8 *buffer_ptr = reinterpret_cast<u8 *>(idt_data);
#define error(_n)                                                              \
  write_gate(buffer_ptr, &_int_handler_vec##_n,                                \
             ((_n) <= 32) ? GateType386::Interrupt : GateType386::Interrupt);  \
  buffer_ptr += 8;
#define noerror(_n) error(_n)
  HANDLERS(noerror, error)
#undef error
#undef noerror
}
} // namespace

namespace kernel::x86 {
void initialize_interrupts() {
  memset(idt_data, 0, idt_entries * 8);
  setup_interrupt_handlers();
  load_idt_table(idt_data, idt_entries * 8);
  printf("Setup %u idt entries at %p!\n", idt_entries, idt_data);
}
} // namespace kernel::x86
