#include <bustd/assert.hpp>
#include <kernel/kmalloc.hpp>
#include <kernel/test_kmalloc.hpp>
#include <libc/stdio.h>
#include <stddef.h>

#define ALIGNMENT_CHECK(x)                                                     \
  ASSERT_EQ(reinterpret_cast<usize>(x) % alignof(max_align_t), 0)

namespace kernel::malloc {
auto AllocTester::run() -> void {
  sanity_check();
  m_alloc.print_allocations();
  printf("[kmalloc] sanity_check() success\n");
  reuse_check();
  m_alloc.print_allocations();
  printf("[kmalloc] reuse_check() success\n");
}

auto AllocTester::sanity_check() -> void {
  auto *const p1 = m_alloc.allocate(1);
  ALIGNMENT_CHECK(p1);
  auto *const p2 = m_alloc.allocate(1);
  ALIGNMENT_CHECK(p2);
  ASSERT_NE(p1, p2);

  m_alloc.deallocate(p2);
  m_alloc.deallocate(p1);
}

auto AllocTester::reuse_check() -> void {
  auto *p1 = m_alloc.allocate(1);
  m_alloc.print_allocations();
  m_alloc.deallocate(p1);
  m_alloc.print_allocations();
  auto *p2 = m_alloc.allocate(1);
  m_alloc.print_allocations();
  ASSERT_EQ(p1, p2);
  m_alloc.deallocate(p2);
  m_alloc.print_allocations();

  p1 = m_alloc.allocate(64);
  m_alloc.print_allocations();
  p2 = m_alloc.allocate(128);
  m_alloc.print_allocations();
  auto *const p3 = m_alloc.allocate(32);
  m_alloc.print_allocations();

  m_alloc.deallocate(p2);
  m_alloc.print_allocations();
  auto *const p4 = m_alloc.allocate(32);
  m_alloc.print_allocations();
  ALIGNMENT_CHECK(p4);
  ASSERT_EQ(p4, p2);

  m_alloc.deallocate(p1);
  m_alloc.deallocate(p3);
  m_alloc.deallocate(p4);

  auto *const p5 = m_alloc.allocate(1);
  ASSERT_EQ(p5, p1);
  m_alloc.deallocate(p1);
}
} // namespace kernel::malloc
