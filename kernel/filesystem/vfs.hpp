#pragma once

#include <bustd/macros.hpp>
#include <bustd/ownedptr.hpp>
#include <bustd/stddef.hpp>
#include <bustd/stringview.hpp>
#include <kernel/filesystem/ifilesystem.hpp>
#include <kernel/filesystem/inode.hpp>
#include <kernel/filesystem/inodeindex.hpp>

namespace kernel {
class Vfs {
  BU_NOCOPY(Vfs)

public:
  static auto instance() -> Vfs &;
  // FIXME: We have to handle file descriptors and such eventually
  auto
  get_inode_at_path(bu::StringView path) -> bu::Optional<filesystem::Inode>;

  auto data_from_inode(const filesystem::InodeIndex &index, u64 offset,
                       usize bytes, u8 *buffer) -> isize;

  auto list_directory(const filesystem::InodeIndex &index)
      -> bu::Optional<bu::Vector<filesystem::DirectoryEntry>>;

  auto quick_read_all_data(bu::StringView path) -> bu::Optional<bu::Vector<u8>>;

  void mount(bu::OwnedPtr<IFileSystem> &&fs);

private:
  Vfs();
  bu::OwnedPtr<IFileSystem> m_root_fs;
};
} // namespace kernel
