#pragma once

#include <kernel/interruptmanager.hpp>
#include <kernel/memory.hpp>

namespace kernel::x86 {

// TODO: bustd should have some generic guard type to make stuff like this
// easier
class PageMapGuard {
public:
  PageMapGuard(PhysicalAddress address);
  ~PageMapGuard();

  void *mapped_address() const;

private:
  InterruptManager::InterruptGuard m_guard;
  static volatile bool s_m_in_use;
};

} // namespace kernel::x86
