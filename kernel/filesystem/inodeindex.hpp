#pragma once

#include <bustd/stddef.hpp>

namespace kernel::filesystem {
class InodeIndex {
public:
  explicit InodeIndex(u64 index);
  InodeIndex() = default;
  ~InodeIndex() = default;
  InodeIndex(const InodeIndex &) = default;
  InodeIndex &operator=(InodeIndex &&) = default;
  InodeIndex(InodeIndex &&) = default;
  InodeIndex &operator=(const InodeIndex &) = default;

  u64 get() const;

private:
  u64 m_index{0};
};
} // namespace kernel::filesystem
