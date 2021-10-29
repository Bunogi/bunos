#include <bustd/assert.hpp>
#include <kernel/panic.hpp>

// ref https://itanium-cxx-abi.github.io/cxx-abi/abi.html

namespace __cxxabiv1 {
// Needed to make pure functions work
extern "C" void __cxa_pure_virtual() {
  KERNEL_PANIC("Called a pure virtual function");
}

// These are only called with local statics before kernel_main(), so we don't
// have to actually use a lock mechanism.
__extension__ typedef int __guard __attribute__((mode(__DI__)));
extern "C" int __cxa_guard_acquire(__guard *const guard) {
  return !*(char *)(guard);
}
extern "C" void __cxa_guard_release(__guard *const guard) {
  *(char *)guard = 1;
}

extern "C" void __cxa_guard_abort(__guard *const) {}

// We will never exit the kernel, so it's safe to have this be a no-op
extern "C" void __cxa_atexit(void (*)(void *), void *, void *) {}

} // namespace __cxxabiv1

// This is intentionally in the kernel so that we can use libssp in GCC in
// userspace.
extern "C" {
[[noreturn]] void __stack_chk_fail() {
  KERNEL_PANIC("Stack smashing detected");
}
}
