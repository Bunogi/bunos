#include <bustd/assert.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/timer.hpp>
#include <kernel/x86/memory.hpp>
#include <kernel/x86/pagemapguard.hpp>
#include <kernel/x86/paging.hpp>
#include <kernel/x86/virtualmemorymap.hpp>

namespace kernel::x86 {
PageMapGuard::PageMapGuard(PhysicalAddress address)
    : m_guard(InterruptManager::instance()->disable_interrupts_guarded()) {
  // FIXME: use spinlock?
  if (s_m_in_use) {
    auto *instance = InterruptManager::instance();
    instance->enable_interrupts();
    while (s_m_in_use) {
      kernel::timer::delay(10);
    }
    s_m_in_use = true;
    instance->disable_interrupts();
  }
  s_m_in_use = true;

  PageTableEntry entry{};
  entry.page_address = address;
  entry.present = true;
  entry.read_write = true;
  constexpr u32 index =
      kernel::vmem::reserved::Temp.to_linked_location().get() / 0x1000;
  // This maps it into memory for *all* processes, because their page
  // directories all point here.
  kernel::x86::kernel_page_table[index] = entry.as_u32();
  _x86_refresh_page_directory();
}

PageMapGuard::~PageMapGuard() {
  ASSERT(s_m_in_use);
  constexpr u32 index =
      kernel::vmem::reserved::Temp.to_linked_location().get() / 0x1000;
  PageTableEntry entry{};
  entry.present = false;
  kernel::x86::kernel_page_table[index] = entry.as_u32();
  _x86_refresh_page_directory();
  s_m_in_use = false;
}

void *PageMapGuard::mapped_address() const {
  return kernel::vmem::reserved::Temp.ptr();
}

volatile bool PageMapGuard::s_m_in_use{false};
} // namespace kernel::x86
