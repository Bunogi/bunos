#include <bustd/assert.hpp>
#include <bustd/stddef.hpp>
#include <bustd/stringview.hpp>
#include <kernel/kmalloc.hpp>
#include <kernel/panic.hpp>
#include <stdio.h>

// VA_OPT is a gnu extension but it exists in C++20 so whatever
#define DEBUG_PRINTF(format, ...)                                              \
  if (m_debug)                                                                 \
  printf("[kmalloc] " format __VA_OPT__(, ) __VA_ARGS__)

extern "C" {
extern char _kernel_heap_start, _kernel_heap_end;
}

static kernel::malloc::Allocator *s_allocator;

namespace kernel::malloc {

// allocation nodes will be split if the capacity of the new node would be at
// least this big
static constexpr usize NODE_SPLIT_THRESHOLD = 8;

#pragma GCC diagnostic push
// It struggles with _kernel_heap_start*/
#pragma GCC diagnostic ignored "-Warray-bounds"
Allocator::Allocator() {
  ASSERT_EQ(s_allocator, nullptr);
  s_allocator = this;

  const auto heap_start = reinterpret_cast<usize>(&_kernel_heap_start);
  ASSERT_EQ(heap_start % BLOCK_SIZE, 0);
  ASSERT_EQ(heap_start % alignof(Block), 0);
  m_first_block = reinterpret_cast<Block *>(&_kernel_heap_start);

  const auto heap_size =
      reinterpret_cast<usize>(&_kernel_heap_end) - heap_start;
  const auto block_count = heap_size / BLOCK_SIZE;
  const auto wasted_space = heap_size % BLOCK_SIZE;
  m_last_block = m_first_block + block_count;

  printf("[kmalloc] Blocksize: %u, first block capacity %u, heap memory region "
         "size: %u. Total blocks: %u (%u KiB). Waste: %u bytes\n",
         BLOCK_SIZE, FIRST_BLOCK_CAPACITY, heap_size, block_count,
         block_count * BLOCK_SIZE / bu::KiB, wasted_space);

  m_first_block->subblocks = block_count - 1;
  m_first_block->free = true;
}
#pragma GCC diagnostic pop

auto Allocator::instance() -> Allocator * {
  ASSERT_NE(s_allocator, nullptr);
  return s_allocator;
}

auto Allocator::allocate(const size_t size) -> void * {
  const auto fits_in_one_block = size < FIRST_BLOCK_CAPACITY;

  const auto size_without_first_block = size - FIRST_BLOCK_CAPACITY;
  const auto one_off = size_without_first_block % BLOCK_SIZE != 0 ? 1 : 0;
  const auto subblocks = size_without_first_block / BLOCK_SIZE + one_off;

  const auto extra_subblocks = fits_in_one_block ? 0 : subblocks;

  auto *block = m_first_block;
  for (; block; block = next_block(block)) {
    if (block->free && block->subblocks >= extra_subblocks) {
      break;
    }
  }

  if (!block) {
    char buffer[256];
    sprintf(buffer, "Not enough heap space for another %u bytes!", size);
    print_allocations();
    KERNEL_PANIC(buffer);
  }

  DEBUG_PRINTF("Allocating %u bytes at %p\n", size, block->data());

  split(block, extra_subblocks);
  block->free = false;
  if (m_debug) {
    print_allocations();
  }
  return block->data();
}

auto Allocator::split(Block *const block, usize extra_subblocks) -> void {
  if (block->subblocks == extra_subblocks) {
    return;
  }
  ASSERT(block->subblocks > extra_subblocks);

  const auto prev_blocks = block->subblocks;
  // next uses this
  block->subblocks = extra_subblocks;

  auto *next = next_block(block);
  if (!next) {
    block->subblocks = prev_blocks;
    return;
  }

  next->subblocks = prev_blocks;
  next->subblocks -= 1;
  next->subblocks -= extra_subblocks;
  next->free = true;
}

auto Allocator::deallocate(void *p) -> void {
  auto *block = m_first_block;
  Block *prev = nullptr;
  for (; block; prev = block, block = next_block(block)) {
    if (block->data() == p) {
      break;
    }
  }
  if (!block) {
    char buf[256];
    sprintf((char *)buf, "Attempt to deallocate unallocated address %p", p);
    print_allocations();
    KERNEL_PANIC(buf);
  }

  DEBUG_PRINTF("Freeing %u blocks at %p\n", block->subblocks + 1,
               block->data());

  block->free = true;
  if (prev && prev->free) {
    // The block we just freed is ahead of another free block.
    // Prevent two free blocks in a row by merging the prev block.
    merge_blocks(prev);
  } else {
    merge_blocks(block);
  }
  if (m_debug) {
    print_allocations();
  }
}

void Allocator::merge_blocks(Block *const block) {

  // If the freed block is between other free blocks, they can all be merged, so
  // keep going as long as possible

  auto *next = next_block(block);
  while (next && next->free) {
    const auto new_blocks = block->subblocks + next->subblocks + 1;
    DEBUG_PRINTF("Merging %p and %p into one with %u subblocks\n", block, next,
                 new_blocks);
    block->subblocks += next->subblocks + 1;
    next = next_block(block);
  }
}

auto Allocator::next_block(Block *const block) -> Block * {
  const auto as_usize = reinterpret_cast<usize>(block);
  const auto block_offset = BLOCK_SIZE * (block->subblocks + 1);

  const auto retval = reinterpret_cast<Block *>(as_usize + block_offset);
  if (retval > m_last_block) {
    return nullptr;
  }
  return retval;
}

auto Allocator::print_allocations() -> void {
  printf("[kmalloc] allocations:\n");
  auto *block = m_first_block;
  u32 i = 0;
  while (block) {
    const char *state = block->free ? "free" : "used";
    const auto bytes = FIRST_BLOCK_CAPACITY + BLOCK_SIZE * block->subblocks;
    printf(
        " %u: block addr: %p, data: %p. subblocks: %u, %u bytes (%uKiB) %s\n",
        i, block, block->data(), block->subblocks, bytes, bytes / bu::KiB,
        state);
    block = next_block(block);
    i++;
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
