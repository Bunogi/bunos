#pragma once

#include <bustd/stddef.hpp>
#include <kernel/filesystem/inode.hpp>

namespace kernel::filesystem {
struct DirectoryEntry {
  char name[256];
  Inode inode;
};
} // namespace kernel::filesystem
