#pragma once

#include <bustd/optional.hpp>
#include <bustd/stddef.hpp>
#include <bustd/stringview.hpp>
#include <bustd/vector.hpp>
#include <kernel/filesystem/directoryentry.hpp>
#include <kernel/filesystem/inode.hpp>

namespace kernel {
class IFileSystem {
public:
  virtual bu::Optional<filesystem::Inode>
  get_inode_at_path(bu::StringView path) = 0;

  // FIXME: Must handle errors better
  // FIXME: needs to take a file descriptor of some kind to know our offset in
  // the file and stuff
  virtual isize data_from_inode(u64 inode_index, u64 offset, usize bytes,
                                u8 *buffer) = 0;
  virtual bu::Optional<bu::Vector<filesystem::DirectoryEntry>>
  list_directory(u64 inode_index) = 0;
};
} // namespace kernel
