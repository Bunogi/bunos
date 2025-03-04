#pragma once

#include <kernel/kmalloc.hpp>

namespace kernel::malloc {

class AllocTester {
public:
  AllocTester(Allocator &alloc) : m_alloc(alloc) {};
  auto run() -> void;

private:
  auto sanity_check() -> void;
  auto reuse_check() -> void;
  auto big_alloc_check() -> void;
  auto block_size_alloc_check() -> void;
  Allocator &m_alloc; // NOLINT
};
} // namespace kernel::malloc
