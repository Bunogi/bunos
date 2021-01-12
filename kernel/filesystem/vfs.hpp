#pragma once

#include <bustd/macros.hpp>
#include <bustd/ownedptr.hpp>
#include <bustd/stddef.hpp>
#include <bustd/stringview.hpp>
#include <kernel/filesystem/ifilesystem.hpp>

namespace kernel {
class Vfs {
  BU_NOCOPY(Vfs)

public:
  static Vfs *instance();
  // FIXME: We have to handle file descriptors and such eventually
  isize read_file(bu::StringView file, u8 *buffer, usize offset, usize len);
  // FIXME: This should be a string type, not a Vector<Vector>
  bu::Vector<bu::Vector<char>> read_dir(bu::StringView path);
  u64 file_size(bu::StringView file);

  void mount(bu::OwnedPtr<IFileSystem> &&fs);

private:
  Vfs();
  bu::OwnedPtr<IFileSystem> m_root_fs;
};
} // namespace kernel
