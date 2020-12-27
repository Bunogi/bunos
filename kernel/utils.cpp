#include <bustd/assert.hpp>

// Needed to make pure functions work
extern "C" void __cxa_pure_virtual() {
  KERNEL_PANIC("Called a pure virtual function");
}
