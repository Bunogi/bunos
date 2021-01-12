#include <bustd/assert.hpp>
#include <kernel/filesystem/vfs.hpp>

namespace kernel {
static Vfs *this_instance;

Vfs *Vfs::instance() {
  if (this_instance == nullptr) {
    this_instance = new Vfs();
  }
  return this_instance;
}

Vfs::Vfs() : m_root_fs(nullptr) {}

void Vfs::mount(bu::OwnedPtr<IFileSystem> &&fs) {
  ASSERT(m_root_fs.is_null());
  m_root_fs = bu::move(fs);
}

bu::Vector<bu::Vector<char>> Vfs::read_dir(bu::StringView path) {
  ASSERT(m_root_fs);
  return m_root_fs->read_dir(path);
}

isize Vfs::read_file(bu::StringView file, u8 *buffer, usize offset, usize len) {
  ASSERT(m_root_fs);
  return m_root_fs->read_file(file, buffer, offset, len);
}

u64 Vfs::file_size(bu::StringView file) { return m_root_fs->file_size(file); }

} // namespace kernel
