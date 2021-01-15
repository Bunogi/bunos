#pragma once

#include <bustd/stddef.hpp>
#include <kernel/memory.hpp>

namespace kernel::pmem {
class PhysicalPage {

public:
  PhysicalPage(PhysicalAddress addr);
  PhysicalAddress address() const;

private:
  PhysicalAddress m_address{PhysicalAddress(0)};
};

void init();

PhysicalPage allocate();
void deallocate(PhysicalPage page);

} // namespace kernel::pmem
