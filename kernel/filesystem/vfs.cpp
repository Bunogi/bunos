#include <bustd/assert.hpp>
#include <bustd/optional.hpp>
#include <kernel/filesystem/vfs.hpp>

namespace kernel {
static Vfs *this_instance;

auto Vfs::instance() -> Vfs & {

  if (this_instance == nullptr) {
    this_instance = new Vfs();
  }
  return *this_instance;
}

Vfs::Vfs() : m_root_fs(nullptr) {}

void Vfs::mount(bu::OwnedPtr<IFileSystem> &&fs) {
  ASSERT(m_root_fs.is_null());
  m_root_fs = bu::move(fs);
}

auto Vfs::get_inode_at_path(const bu::StringView path)
    -> bu::Optional<filesystem::Inode> {
  ASSERT(m_root_fs);
  return m_root_fs->get_inode_at_path(path);
}

auto Vfs::data_from_inode(const filesystem::InodeIndex &index, const u64 offset,
                          const usize bytes, u8 *const buffer) -> isize {
  ASSERT(m_root_fs);

  // FIXME: check index for which file system it belongs to

  return m_root_fs->data_from_inode(index.get(), offset, bytes, buffer);
}

auto Vfs::quick_read_all_data(const bu::StringView path)
    -> bu::Optional<bu::Vector<u8>> {
  const auto inode = get_inode_at_path(path);
  if (!inode) {
    return bu::create_none<bu::Vector<u8>>();
  }
  ASSERT_EQ(inode->type, filesystem::InodeType::File);

  bu::Vector<u8> out(inode->file_size);
  out.fill('\0', inode->file_size);

  const auto read =
      data_from_inode(inode->index, 0, inode->file_size, out.data());
  ASSERT_EQ(static_cast<u64>(read), inode->file_size);

  return bu::create_some<bu::Vector<u8>>(out);
}

auto Vfs::list_directory(const filesystem::InodeIndex &index)
    -> bu::Optional<bu::Vector<filesystem::DirectoryEntry>> {
  ASSERT(m_root_fs);
  // FIXME: have to check where it came from
  return m_root_fs->list_directory(index.get());
}

} // namespace kernel
