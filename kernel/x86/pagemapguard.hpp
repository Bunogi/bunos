#pragma once

#include <bustd/macros.hpp>
#include <kernel/memory.hpp>
#include <kernel/spinlock.hpp>

namespace kernel::x86 {

// TODO: bustd should have some generic guard type to make stuff like this
// easier
class PageMapGuard {
  BU_NOCOPY(PageMapGuard)
  BU_NOMOVE(PageMapGuard)
public:
  PageMapGuard(PhysicalAddress address);
  ~PageMapGuard();

  [[nodiscard]] auto mapped_address() const -> void *;

private:
  static SpinLock s_m_lock;
  SpinLock::Guard m_lock_guard;
};

} // namespace kernel::x86
