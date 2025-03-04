#include <bustd/assert.hpp>
#include <kernel/kmalloc.hpp>
#include <kernel/test_kmalloc.hpp>
#include <libc/stdio.h>
#include <stddef.h>

#define ALIGNMENT_CHECK(x)                                                     \
  ASSERT_EQ(reinterpret_cast<usize>(x) % alignof(max_align_t), 0)

namespace kernel::malloc {
auto AllocTester::run() -> void {
  const auto free_subblocks_before = m_alloc.m_first_block->subblocks;
  sanity_check();
  printf("[kmalloc] sanity_check() success\n");
  ASSERT_EQ(free_subblocks_before, m_alloc.m_first_block->subblocks);

  block_size_alloc_check();
  ASSERT_EQ(free_subblocks_before, m_alloc.m_first_block->subblocks);
  printf("[kmalloc] block_size_alloc_check() success\n");

  reuse_check();
  ASSERT_EQ(free_subblocks_before, m_alloc.m_first_block->subblocks);
  printf("[kmalloc] reuse_check() success\n");

  big_alloc_check();
  ASSERT_EQ(free_subblocks_before, m_alloc.m_first_block->subblocks);
  printf("[kmalloc] big_alloc_check() success\n");
}

auto AllocTester::block_size_alloc_check() -> void {
  auto *const p1 = m_alloc.allocate(m_alloc.BLOCK_SIZE);
  ALIGNMENT_CHECK(p1);
  m_alloc.deallocate(p1);
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
  m_alloc.deallocate(p1);
  auto *p2 = m_alloc.allocate(1);
  ASSERT_EQ(p1, p2);
  m_alloc.deallocate(p2);

  p1 = m_alloc.allocate(64);
  p2 = m_alloc.allocate(128);
  auto *const p3 = m_alloc.allocate(32);

  m_alloc.deallocate(p2);
  auto *const p4 = m_alloc.allocate(32);
  ALIGNMENT_CHECK(p4);
  ASSERT_EQ(p4, p2);

  m_alloc.deallocate(p1);
  m_alloc.deallocate(p3);
  m_alloc.deallocate(p4);

  auto *const p5 = m_alloc.allocate(1);
  ASSERT_EQ(p5, p1);
  m_alloc.deallocate(p1);
}

auto AllocTester::big_alloc_check() -> void {
  auto *const p1 = m_alloc.allocate(512);
  auto *const p2 = m_alloc.allocate(1024);
  m_alloc.deallocate(p1);
  auto *p3 = m_alloc.allocate(128);
  ASSERT_EQ(p1, p3);
  m_alloc.deallocate(p1);
  m_alloc.deallocate(p2);
}

auto AllocTester::many_allocations_check() -> void {
  constexpr auto pointer_count = 512;
  void *pointers[pointer_count];
  for (auto &pointer : pointers) {
    pointer = m_alloc.allocate(16);
  }

  for (auto &pointer : pointers) {
    m_alloc.deallocate(pointer);
  }
}
} // namespace kernel::malloc
