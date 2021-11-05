#pragma once

#include <bustd/stddef.hpp>

namespace kernel::filesystem {

enum class InodeType { Directory, File };

struct Inode {
  u64 index;
  u64 file_size;
  InodeType type;
};
} // namespace kernel::filesystem
