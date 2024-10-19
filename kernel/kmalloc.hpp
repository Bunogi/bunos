#pragma once

#include <bustd/stddef.hpp>

namespace kernel::malloc {
class Allocator {
public:
  explicit Allocator();
  static auto instance() -> Allocator *;
  auto allocate(size_t size) -> void *;
  void deallocate(void *p);

  void print_allocations();

private:
  enum class State { Used, Free };
  struct Node {
    Node *next, *prev;
    usize capacity;
    State state;
  };

  auto previously_allocated(void *p) -> bool;
  auto is_allocated_in_node(const void *p, const Node *const node) -> bool;
  void try_merge_free_nodes(Node *node);

  uintptr_t m_heap_start;
  uintptr_t m_heap_end;
  Node *m_alloc_head;
  // Align everything by 4-bytes because all types have to be properly aligned.
  static constexpr u32 m_data_offset{sizeof(Node) + sizeof(Node) % 8};
};
} // namespace kernel::malloc
