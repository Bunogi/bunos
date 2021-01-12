#include <stdlib.h>

#ifdef __IN_KERNEL__
#include <kernel/kmalloc.hpp>
#endif

void *malloc(size_t size) {
#ifdef __IN_KERNEL__
  return kernel::malloc::Allocator::instance()->allocate(size);
#else
#error Missing malloc implementation in userspace
#endif
}

void free(void *ptr) {
#ifdef __IN_KERNEL__
  return kernel::malloc::Allocator::instance()->deallocate(ptr);
#else
#error Missing free implementation in userspace
#endif
}
