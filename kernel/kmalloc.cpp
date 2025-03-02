#include <bustd/assert.hpp>
#include <bustd/stringview.hpp>
#include <kernel/kmalloc.hpp>
#include <kernel/panic.hpp>
#include <stdio.h>

#define KMALLOC_DEBUG

#ifdef KMALLOC_DEBUG
// VA_OPT is a gnu extension but it exists in C++20 so whatever
#define DEBUG_PRINTF(format, ...)                                              \
  printf("[kmalloc] " format __VA_OPT__(, ) __VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif

extern "C" {
extern void *_kernel_heap_start, *_kernel_heap_end;
}

static kernel::malloc::Allocator *s_allocator;

namespace kernel::malloc {

// allocation nodes will be split if the capacity of the new node would be at
// least this big
static constexpr usize NODE_SPLIT_THRESHOLD = 8;

#pragma GCC diagnostic push
// It struggles with _kernel_heap_start
#pragma GCC diagnostic ignored "-Warray-bounds"
Allocator::Allocator()
    : m_heap_start(reinterpret_cast<uintptr_t>(&_kernel_heap_start)),
      m_heap_end(reinterpret_cast<uintptr_t>(&_kernel_heap_end)) {
  ASSERT_EQ(s_allocator, nullptr);

  m_alloc_head = reinterpret_cast<Node *>(m_heap_start);
  ASSERT(reinterpret_cast<usize>(m_alloc_head) % alignof(Node) == 0);
  m_alloc_head->capacity = m_heap_end - m_heap_start - offsetof(Node, dummy);
  m_alloc_head->state = State::Free;
  m_alloc_head->next = nullptr;
  s_allocator = this;
}
#pragma GCC diagnostic pop

auto Allocator::instance() -> Allocator * {
  ASSERT_NE(s_allocator, nullptr);
  return s_allocator;
}

auto Allocator::allocate(size_t size) -> void * {
  ASSERT_NE(size, 0);
  Node *node = m_alloc_head;

  // Find the first region with enough space
  while (node) {
    if (node->state == State::Free && node->capacity >= size) {
      break;
    }

    node = node->next;
  }
  if (!node) {
    KERNEL_PANIC("Ran out of heap space!");
  }

  split(node, size);

  node->state = State::Used;

  void *retval = &node->dummy;
  DEBUG_PRINTF("Allocated %d bytes at %p\n", size, retval);

  /*
  const auto data_addr =
      reinterpret_cast<usize>(node) + NODE_DATA_PADDING_BYTES;
  return reinterpret_cast<void *>(data_addr);
  */
  return retval;
}

auto Allocator::split(Node *node, usize new_size) -> void {
  /*
  const auto node_usize = reinterpret_cast<usize>(node);

  const auto next_address = node_usize + sizeof(Node) + new_size;
  const auto extra_align = next_address % alignof(Node);
  const auto missing_align = alignof(max_align_t) - extra_align;
  const auto next_aligned_address = next_address + missing_align;
  const auto next_data_address = next_aligned_address + NODE_DATA_PADDING_BYTES;
  ASSERT_EQ(next_aligned_address % alignof(Node), 0);
  ASSERT_EQ(next_data_address % alignof(max_align_t), 0);
  static_assert(alignof(max_align_t) == 8);

  const auto data_bytes_in_next_node = next_data_address - next_address;
  // You could probably performance tune this a lot...
  if (data_bytes_in_next_node < NODE_SPLIT_THRESHOLD) {
    return;
  }
  */

  // FIXME: Maybe a safety margin with canary would be good?
  //
  const auto data_offset = reinterpret_cast<usize>(&node->dummy);
  const auto next_address = data_offset + new_size;
  const auto missing_align = alignof(Node) - (next_address % alignof(Node));
  const auto next_aligned_address = next_address + missing_align;

  ASSERT_EQ(next_aligned_address % alignof(Node), 0);
  Node *next = reinterpret_cast<Node *>(next_aligned_address);
  const auto next_data_address = reinterpret_cast<usize>(&next->dummy);
  ASSERT_EQ(next_data_address % alignof(max_align_t), 0);

  const auto bytes_to_next_data = next_data_address - data_offset;
  const auto bytes_in_next_node = node->capacity - bytes_to_next_data;

  // You could probably performance tune this a lot...
  if (bytes_in_next_node < NODE_SPLIT_THRESHOLD) {
    return;
  }

  // NOTE: This will be null when this is the last node
  Node *old_next = node->next;
  next->capacity = bytes_in_next_node;
  next->next = old_next;
  next->state = State::Free;
  node->next = next;
  node->capacity = new_size;
}

auto Allocator::deallocate(void *p) -> void {
  auto *node = m_alloc_head;
  Node *prev = nullptr;
  while (node) {
    if (p != &node->dummy) {
      prev = node;
      node = node->next;
      continue;
    }

    node->state = State::Free;

    DEBUG_PRINTF("Deallocated %u bytes at %p\n", node->capacity, &node->dummy);

    if (!prev) {
      merge_nodes_forward(node);
    } else if (prev && prev->state == State::Free) {
      merge_nodes_forward(prev);
    }

    return;
  }

  char error[256];
  // FIXME: snprintf
  sprintf(error, "KMalloc: De-alloc of unallocated pointer %p", p);
  KERNEL_PANIC(error);
}

auto Allocator::is_allocated_in_node(const void *p, const Node *const node)
    -> bool {
  return p == &node->dummy;
}

void Allocator::merge_nodes_forward(Node *node) {
  if (node->state != State::Free) {
    return;
  }

  // If the freed node is between othre free nodes, they can all be merged, so
  // keep going as long as possible
  while (node->next && node->next->state == State::Free) {
    const auto this_data = reinterpret_cast<usize>(&node->dummy);
    const auto next_data = reinterpret_cast<usize>(&node->next->dummy);
    const auto reclaimable_space = next_data - this_data + node->next->capacity;
    const auto new_capacity = node->capacity + reclaimable_space;
    DEBUG_PRINTF("Merging %p and %p into one of capacity %u\n", node,
                 node->next, new_capacity);
    node->capacity = new_capacity;
    node->next = node->next->next;
  }
}

auto Allocator::previously_allocated(void *p) -> bool {
  auto *node = m_alloc_head;
  while (node != nullptr) {
    if (is_allocated_in_node(p, node)) {
      return true;
    }
    node = node->next;
  }
  return false;
}

auto Allocator::print_allocations() -> void {
  printf("[kmalloc]: allocations:\n");
  auto *node = m_alloc_head;
  while (node) {
    const char *state = node->state == State::Free ? "free" : "used";
    printf("  %p: %u bytes %s\n", &node->dummy, node->capacity, state);
    node = node->next;
  }
}
} // namespace kernel::malloc

auto operator new(size_t size) -> void * {
  ASSERT_NE(s_allocator, nullptr);
  return s_allocator->allocate(size);
}

auto operator new[](size_t size) -> void * {
  ASSERT_NE(s_allocator, nullptr);
  return s_allocator->allocate(size);
}

void operator delete(void *p) {
  ASSERT_NE(s_allocator, nullptr);
  s_allocator->deallocate(p);
}

void operator delete[](void *p) {
  ASSERT_NE(s_allocator, nullptr);
  s_allocator->deallocate(p);
}

// Called to in theory be faster, but we can safely ignore this one
void operator delete(void *p, size_t) {
  ASSERT_NE(s_allocator, nullptr);
  s_allocator->deallocate(p);
}
void operator delete[](void *p, size_t) {
  ASSERT_NE(s_allocator, nullptr);
  s_allocator->deallocate(p);
}
