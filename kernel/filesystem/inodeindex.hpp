#pragma once

#include <bustd/stddef.hpp>

namespace kernel::filesystem {
class InodeIndex {
public:
  explicit InodeIndex(u64 index);
  InodeIndex() = default;
  ~InodeIndex() = default;
  InodeIndex(const InodeIndex &) = default;
  auto operator=(InodeIndex &&) -> InodeIndex & = default;
  InodeIndex(InodeIndex &&) = default;
  auto operator=(const InodeIndex &) -> InodeIndex & = default;

  auto get() const -> u64;

private:
  u64 m_index{0};
};
} // namespace kernel::filesystem
