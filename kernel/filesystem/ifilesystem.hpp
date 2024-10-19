#pragma once

#include <bustd/macros.hpp>
#include <bustd/optional.hpp>
#include <bustd/stddef.hpp>
#include <bustd/stringview.hpp>
#include <bustd/vector.hpp>
#include <kernel/filesystem/directoryentry.hpp>
#include <kernel/filesystem/inode.hpp>

namespace kernel {
class IFileSystem {
  BU_NOCOPY(IFileSystem)
  BU_NOMOVE(IFileSystem)
public:
  virtual auto
  get_inode_at_path(bu::StringView path) -> bu::Optional<filesystem::Inode> = 0;

  IFileSystem() = default;
  virtual ~IFileSystem() = default;

  // FIXME: Must handle errors better
  // FIXME: needs to take a file descriptor of some kind to know our offset in
  // the file and stuff
  virtual auto data_from_inode(u64 inode_index, u64 offset, usize bytes,
                               u8 *buffer) -> isize = 0;
  virtual auto list_directory(u64 inode_index)
      -> bu::Optional<bu::Vector<filesystem::DirectoryEntry>> = 0;
};
} // namespace kernel
