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

  auto debug(const bool f) -> void { m_debug = f; }

  void print_allocations();

  friend class AllocTester;

private:
  constexpr static usize BLOCK_SIZE = 32;

  struct Block {
    bool free;
    // TODO: Here we can do some rudimentary buffer overflow detection, by
    // storing the number of bytes actually allocated and filling with a canary.
    usize subblocks;

    auto data() -> void * {
      // evil casting funtimes
      const auto retval =
          reinterpret_cast<usize>(this) + offsetof(AlignedBlock, align);
      return reinterpret_cast<void *>(retval);
    }
  };

  struct AlignedBlock {
    Block b;
    max_align_t align;
  };

  constexpr static auto FIRST_BLOCK_CAPACITY =
      BLOCK_SIZE - offsetof(AlignedBlock, align);
  static_assert(FIRST_BLOCK_CAPACITY >= BLOCK_SIZE / 2,
                "Block size too small!");

  void merge_blocks(Block *block);
  void split(Block *, usize subblocks);

  auto next_block(Block *) -> Block *;

  Block *m_first_block;
  Block *m_last_block;
  bool m_debug{false};
};
} // namespace kernel::malloc
