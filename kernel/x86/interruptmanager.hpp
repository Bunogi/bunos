#pragma once

#include <bustd/macros.hpp>
#include <kernel/x86/interrupts.hpp>

namespace kernel::interrupt::x86 {
class InterruptManager {
  BU_NOCOPY(InterruptManager)
public:
  static InterruptManager *instance();
  // TODO: Remove both of these when we have a kmalloc()
  static void init(InterruptManager *instance);
  InterruptManager();

  // returns ID of that handler
  // handler function: Returns whether or not the interrupt was handled
  typedef bool (*InterruptHandler)(InterruptFrame *frame);
  u16 register_handler(u16 vector, InterruptHandler handler);
  void unregister_handler(usize handler_id); // maybe unneeded?

  bool handle_interrupt(InterruptFrame *frame);

  bool interrupts_enabled();

  void disable_interrupts();
  void enable_interrupts();

  void disable_non_printing_interrupts();

  class InterruptGuard {
    friend class InterruptManager;
    BU_NOCOPY(InterruptGuard)
  public:
    ~InterruptGuard();
    bool disabled_interrupts() const;

  private:
    InterruptManager *m_parent;
    bool m_did_disable_interrupts{false};
    explicit InterruptGuard(InterruptManager *parent);
  };

  InterruptGuard disable_interrupts_guarded();

private:
  // TODO: Allow multiple listeners on the same interrupt?
  InterruptHandler m_handlers[256]{};
};
} // namespace kernel::interrupt::x86
