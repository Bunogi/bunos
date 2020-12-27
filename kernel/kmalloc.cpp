#include <bustd/assert.hpp>
#include <kernel/kmalloc.hpp>
#include <stdio.h>

//#define KMALLOC_DEBUG

#ifdef KMALLOC_DEBUG
// VA_OPT is a gnu extension but it exists in C++20 so whatever
#define DEBUG_PRINTF(format, ...)                                              \
  printf("[kmalloc] " format __VA_OPT__(, ) __VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif

extern "C" {
extern char _kernel_heap_start, _kernel_heap_end;
}

static kernel::malloc::Allocator *allocator;

namespace kernel::malloc {

Allocator::Allocator()
    : m_heap_start(reinterpret_cast<uintptr_t>(&_kernel_heap_start)),
      m_heap_end(reinterpret_cast<uintptr_t>(&_kernel_heap_end)) {
  ASSERT_EQ(allocator, nullptr);

  m_alloc_head = reinterpret_cast<Node *>(m_heap_start);
  m_alloc_head->capacity = m_heap_end - m_heap_start - m_data_offset;
  m_alloc_head->state = State::Free;
  m_alloc_head->next = m_alloc_head->prev = nullptr;
  allocator = this;
}

void *Allocator::allocate(size_t size) {
  // Find the first region with enough space
  Node *current_node = m_alloc_head;
  Node *node_with_space = nullptr;

  while (current_node != nullptr) {
    if (current_node->state == State::Free && current_node->capacity >= size) {
      node_with_space = current_node;
      break;
    }
    current_node = current_node->next;
  }
  ASSERT_NE(node_with_space, nullptr); // out of heap memory

  node_with_space->state = State::Used;
  const auto margin = node_with_space->capacity - size;
  // Split the node in two
  if (margin != 0) {
    DEBUG_PRINTF("Found node with space at 0x%.8X, with %u bytes capacity\n",
                 node_with_space, node_with_space->capacity);
    const uintptr_t new_node_addr =
        reinterpret_cast<uintptr_t>(node_with_space) + size + m_data_offset;
    Node *const new_node = reinterpret_cast<Node *>(new_node_addr);
    new_node->state = State::Free;
    new_node->capacity = node_with_space->capacity - m_data_offset;
    new_node->next = node_with_space->next;
    node_with_space->next = new_node;
    node_with_space->capacity = size;
    DEBUG_PRINTF("Took %u bytes from node at %p with next %p\n", size,
                 node_with_space, node_with_space->next);
    DEBUG_PRINTF("New node addr: %p with next %p and capacity %u\n", new_node,
                 new_node->next, new_node->capacity);
  }
  DEBUG_PRINTF("Allocated %u bytes at %p\n", size, node_with_space);

  return reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(node_with_space) +
                                  m_data_offset);
}

void Allocator::deallocate(void *p) {
  // For fun, check that we actually allocated this, for debugging purposes.
  // This is actually undefined behaviour, so we can do whatever
  ASSERT(previously_allocated(p));

  // If the data is in the head, we don't have a previous node to change
  if (is_allocated_in_node(p, m_alloc_head)) {
    m_alloc_head->state = State::Free;
    try_merge_free_nodes(m_alloc_head);
    return;
  }

  auto *prev_node = m_alloc_head;
  while (prev_node->next != nullptr) {
    if (is_allocated_in_node(p, prev_node->next)) {
      prev_node->next->state = State::Free;
      try_merge_free_nodes(prev_node->next);
      try_merge_free_nodes(prev_node);
      return;
    }
    prev_node = prev_node->next;
  }

  UNREACHABLE();
}

bool Allocator::is_allocated_in_node(const void *p, const Node *const node) {
  const auto ptr = reinterpret_cast<uintptr_t>(p);
  if (reinterpret_cast<uintptr_t>(node) == ptr - m_data_offset) {
    DEBUG_PRINTF("%p was allocated at %p\n", p, node);
    return true;
  } else {
    return false;
  }
}

void Allocator::try_merge_free_nodes(Node *node) {
  ASSERT_EQ(node->state, State::Free);

  if (node->next != nullptr && node->next->state == State::Free) {
    const auto old_capacity = node->capacity;
    // Reclaim the internal data
    const auto new_capacity =
        old_capacity + node->next->capacity + m_data_offset;
    DEBUG_PRINTF("Merging %p and %p into one of capacity %u\n", node,
                 node->next, new_capacity);
    node->capacity += new_capacity;
    node->next = node->next->next;
  }
}

bool Allocator::previously_allocated(void *p) {
  DEBUG_PRINTF("Checking %p\n", p);
  auto *node = m_alloc_head;
  while (node != nullptr) {
    if (is_allocated_in_node(p, node)) {
      return true;
    }
    DEBUG_PRINTF("THIS: %p\n", node);
    node = node->next;
  }
  return false;
}

void Allocator::print_allocations() {
  auto *node = m_alloc_head;
  printf("[kmalloc] ===> Print allocations start...\n");
  while (node != nullptr) {
    bu::StringView status = node->state == State::Free ? "Free" : "Used";
    printf("[kmalloc] Node %p: capacity %u, status: %s\n", node, node->capacity,
           status.data());
    node = node->next;
  }
  printf("[kmalloc] ===> Print allocations stop\n");
}

} // namespace kernel::malloc

void *operator new(size_t size) {
  ASSERT_NE(allocator, nullptr);
  return allocator->allocate(size);
}

void *operator new[](size_t size) {
  ASSERT_NE(allocator, nullptr);
  return allocator->allocate(size);
}

void operator delete(void *p) {
  ASSERT_NE(allocator, nullptr);
  allocator->deallocate(p);
}

void operator delete[](void *p) {
  ASSERT_NE(allocator, nullptr);
  allocator->deallocate(p);
}

// Called to in theory be faster, but we can safely ignore this one
void operator delete(void *p, size_t) {
  ASSERT_NE(allocator, nullptr);
  allocator->deallocate(p);
}
void operator delete[](void *p, size_t) {
  ASSERT_NE(allocator, nullptr);
  allocator->deallocate(p);
}
