#pragma once

#include <stddef.h>

// placement new and placement delete. Very rarely needed
inline auto operator new(size_t, void *p) throw() -> void * { return p; }
inline auto operator new[](size_t, void *p) throw() -> void * { return p; }
inline void operator delete(void *, void *)throw() {}
inline void operator delete[](void *, void *) throw() {}
