#include <bustd/assert.hpp>
#include <kernel/panic.hpp>

// Needed to make pure functions work
extern "C" void __cxa_pure_virtual() {
  KERNEL_PANIC("Called a pure virtual function");
}

// This is intentionally in the kernel so that we can use libssp in GCC in
// userspace.
extern "C" {
[[noreturn]] void __stack_chk_fail() {
  KERNEL_PANIC("Stack smashing detected");
}
}
