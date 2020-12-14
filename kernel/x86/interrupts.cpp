#include <bustd/stddef.hpp>

#include <stdio.h>
#include <string.h>

#include "interrupts.hpp"
#include <kernel/panic.hpp>

#include "handlers.inc"

// Stuff from interrupts.S
extern "C" {
extern void load_idt_table(u8 *address, u16 length);

#define error(_n) extern void _int_handler_vec##_n();
#define noerror(_n) error(_n)
HANDLERS(error, noerror)
#undef error
#undef noerror
}

namespace {
namespace Local {
enum class GateType386 : u8 { Task = 0x5, Interrupt = 0xE, Trap = 0xF };

// Interrupt descriptor table data
constexpr u16 idt_entries = 256;
// Preallocate space for all entries
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
  u8 *buffer_ptr = reinterpret_cast<u8 *>(idt_data);
#define error(_n)                                                              \
  write_gate(buffer_ptr, &_int_handler_vec##_n, GateType386::Trap);            \
  buffer_ptr += 8;
#define noerror(_n) error(_n)
  HANDLERS(noerror, error)
#undef error
#undef noerror
}
} // namespace Local
} // namespace
extern "C" {
// Interrupt-handler callable functions
//

void _isr_callable_error_code(kernel::interrupt::x86::InterruptFrame *frame) {
  kernel::panic_from_interrupt(frame, nullptr, true);
}

void _isr_callable_noerror(kernel::interrupt::x86::InterruptFrame *frame) {
  kernel::panic_from_interrupt(frame, nullptr, false);
}
}

namespace kernel::interrupt::x86 {
void initialize() {
  memset(Local::idt_data, 0, Local::idt_entries * 8);
  Local::setup_interrupt_handlers();
  load_idt_table(Local::idt_data, Local::idt_entries * 8);
  printf("Setup %u idt entries at %p!\n", Local::idt_entries, Local::idt_data);
}
} // namespace kernel::interrupt::x86
