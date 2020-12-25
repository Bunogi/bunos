#include <kernel/x86/pic.hpp>

#include <bustd/assert.hpp>
#include <bustd/stddef.hpp>
#include <kernel/x86/io.hpp>

// TODO: remove
#include <stdio.h>

namespace kernel::interrupt::x86::pic {
static constexpr u16 master_pic_csr = 0x0020;
static constexpr u16 master_pic_imr = 0x0021;

static constexpr u16 slave_pic_csr = 0x00A0;
static constexpr u16 slave_pic_imr = 0x00A1;

// IC4: 1, SNGL: 0, ADI: 0, LTIM: 0(edge trigger)
// init: 1, rest: must be 0 on x86
static constexpr u8 icw1 = 0x11;

// Vectors <32 are reserved for exceptions in protected mode, so map the IRQs
// above that: 0x20-0x28 for the master, 0x28-0x30 for the slave.
static constexpr u8 master_icw2 = master_vector_offset;
static constexpr u8 slave_icw2 = slave_vector_offset;

// bit n decides that pin n is used to cascade to the slave
// Use IRQ line 2
static constexpr u8 master_icw3 = 0x04;
// 010 => IRQ line 2
static constexpr u8 slave_icw3 = 0x02;

// 80x86 mode as opposed to other weird systems
static constexpr u8 icw4 = 0x01;

void initialize() {
  using kernel::x86::io::out_u8;
  // Init sequence
  out_u8(master_pic_csr, icw1);
  out_u8(slave_pic_csr, icw1);

  out_u8(master_pic_imr, master_icw2);
  out_u8(slave_pic_imr, slave_icw2);

  out_u8(master_pic_imr, master_icw3);
  out_u8(slave_pic_imr, slave_icw3);

  out_u8(master_pic_imr, icw4);
  out_u8(slave_pic_imr, icw4);

  // Disable all interrupts for now
  out_u8(master_pic_imr, 0xFF);
  out_u8(slave_pic_imr, 0xFF);
}

void unmask_irq(u8 irq) {
  using kernel::x86::io::in_u8;
  using kernel::x86::io::out_u8;
  ASSERT(irq < 16);
  // slave pic
  if (irq >= 8) {
    // Ensure that we can cascade the interrupt
    unmask_irq(2);
    u8 irqs = in_u8(slave_pic_imr);
    irqs |= ~(1 << (irq - 8));
    out_u8(slave_pic_imr, irqs);
  } else {
    u8 irqs = in_u8(master_pic_imr);
    printf("0x%.2x ", irqs);
    irqs &= ~(1 << irq);
    printf("0x%.2x\n", irqs);
    out_u8(master_pic_imr, irqs);
  }
}

void acknowledge(u8 irq) {
  ASSERT(irq < 16);

  if (irq >= 8) {
    kernel::x86::io::out_u8(slave_pic_csr, 0x20);
  }
  kernel::x86::io::out_u8(master_pic_csr, 0x20);
}

} // namespace kernel::interrupt::x86::pic