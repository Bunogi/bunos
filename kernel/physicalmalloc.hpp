#pragma once

#include <bustd/stddef.hpp>

namespace kernel::pmem {
class PhysicalPage {

public:
  PhysicalPage(u64 addr);
  u64 address() const;

private:
  u64 m_address{0};
};

void init();

PhysicalPage allocate();
void deallocate(PhysicalPage page);

} // namespace kernel::pmem
