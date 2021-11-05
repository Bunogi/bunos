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
  static Vfs &instance();
  // FIXME: We have to handle file descriptors and such eventually
  bu::Optional<filesystem::Inode> get_inode_at_path(bu::StringView path);

  isize data_from_inode(const filesystem::InodeIndex &index, u64 offset,
                        usize bytes, u8 *buffer);

  bu::Optional<bu::Vector<filesystem::DirectoryEntry>>
  list_directory(const filesystem::InodeIndex &index);

  bu::Optional<bu::Vector<u8>> quick_read_all_data(bu::StringView path);

  void mount(bu::OwnedPtr<IFileSystem> &&fs);

private:
  Vfs();
  bu::OwnedPtr<IFileSystem> m_root_fs;
};
} // namespace kernel
