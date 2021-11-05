#pragma once

#include <bustd/stddef.hpp>
#include <kernel/filesystem/inodeindex.hpp>

namespace kernel::filesystem {

enum class InodeType { Directory, File };

struct Inode {
  InodeIndex index;
  u64 file_size;
  InodeType type;
};
} // namespace kernel::filesystem
