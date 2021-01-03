#pragma once

#include <bustd/macros.hpp>
#include <kernel/x86/interrupts.hpp>

namespace kernel {
class InterruptManager {
  BU_NOCOPY(InterruptManager)
public:
  static InterruptManager *instance();
  // TODO: Remove both of these when we have a kmalloc()
  static void init(InterruptManager *instance);
  InterruptManager();

  u16 register_handler(u16 vector, x86::InterruptHandler handler);
  void unregister_handler(usize handler_id); // maybe unneeded?

  bool handle_interrupt(x86::InterruptFrame *frame);

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
  x86::InterruptHandler m_handlers[256]{};
};
} // namespace kernel
