#pragma once

#include <bustd/macros.hpp>

namespace kernel {
class InterruptGuard {
  BU_NOCOPY(InterruptGuard)
  BU_NOMOVE(InterruptGuard)
public:
  explicit InterruptGuard();
  ~InterruptGuard();
  [[nodiscard]] auto disabled_interrupts() const -> bool;

private:
  bool m_did_disable_interrupts{false};
};

} // namespace kernel
