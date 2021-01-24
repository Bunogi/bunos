#include <bustd/assert.hpp>
#include <kernel/interruptmanager.hpp>
#include <kernel/x86/pic.hpp>
#include <string.h>

namespace kernel {
static InterruptManager *this_instance;

InterruptManager *InterruptManager::instance() {
  ASSERT_NE(this_instance, nullptr);
  return this_instance;
}

InterruptManager::InterruptManager() { x86::pic::initialize(); }

void InterruptManager::init(InterruptManager *instance) {
  this_instance = instance;
}

u16 InterruptManager::register_handler(u16 vector,
                                       x86::InterruptHandler handler) {
  constexpr u8 usable_vector_offset = 32;
  // Reserved for exceptions
  ASSERT(vector >= usable_vector_offset);
  // Currently we have no way to add multiple listeners(TODO: Should we?)
  ASSERT_EQ(m_handlers[vector], nullptr);

  m_handlers[vector] = handler;

  // FROM PIC
  if (vector >= usable_vector_offset &&
      vector < x86::pic::slave_vector_offset + 8) {
    x86::pic::unmask_irq(vector - usable_vector_offset);
  }

  return vector;
}

void InterruptManager::enable_interrupts() { __asm__ volatile("sti"); }
void InterruptManager::disable_interrupts() { __asm__ volatile("cli"); }
bool InterruptManager::interrupts_enabled() {
  u32 eflags;
  __asm__ volatile("pushf\n"
                   "popl %0"
                   : "=r"(eflags));
  return (eflags & 0x200) != 0;
}

bool InterruptManager::handle_interrupt(x86::InterruptFrame *frame) {
  if (m_handlers[frame->int_vector] != nullptr) {
    const auto retval = m_handlers[frame->int_vector](frame);
    if (retval && frame->int_vector < 0x30) {
      x86::pic::acknowledge(frame->int_vector - 0x20);
    }
    return retval;
  } else {
    return false;
  }
}

InterruptManager::InterruptGuard
InterruptManager::disable_interrupts_guarded() {
  return InterruptGuard(this);
}

InterruptManager::InterruptGuard::InterruptGuard(InterruptManager *parent)
    : m_parent(parent) {
  if (parent->interrupts_enabled()) {
    m_did_disable_interrupts = true;
    m_parent->disable_interrupts();
  }
}

InterruptManager::InterruptGuard::~InterruptGuard() {
  if (m_did_disable_interrupts) {
    m_parent->enable_interrupts();
  }
}

bool InterruptManager::InterruptGuard::disabled_interrupts() const {
  return m_did_disable_interrupts;
}

void InterruptManager::disable_non_printing_interrupts() {
  x86::pic::mask_non_printing_irqs();
}
} // namespace kernel
