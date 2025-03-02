#pragma once

#include <bustd/stddef.hpp>
#include <stddef.h>

namespace kernel::malloc {

class AllocTester;

class Allocator {
public:
  explicit Allocator();
  static auto instance() -> Allocator *;
  auto allocate(size_t size) -> void *;
  void deallocate(void *p);

  void print_allocations();

  friend class AllocTester;

private:
  enum class State { Used, Free };
  struct Node {
    Node *next;
    usize capacity;
    State state;

    // Contains the user data. Used to ensure the data is aligned
    max_align_t dummy;
  };

  auto previously_allocated(void *p) -> bool;
  auto is_allocated_in_node(const void *p, const Node *const node) -> bool;
  void merge_nodes_forward(Node *node);
  void split(Node *, usize);

  uintptr_t m_heap_start;
  uintptr_t m_heap_end;
  Node *m_alloc_head;
};
} // namespace kernel::malloc
