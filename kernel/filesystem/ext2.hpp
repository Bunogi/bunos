#pragma once

#include <bustd/iteration.hpp>
#include <bustd/macros.hpp>
#include <bustd/ownedptr.hpp>
#include <bustd/stddef.hpp>
#include <bustd/stringview.hpp>
#include <bustd/vector.hpp>
#include <kernel/filesystem/ext2_structs.hpp>
#include <kernel/filesystem/ifilesystem.hpp>

namespace kernel::fs {
class Ext2 : public IFileSystem {
  BU_NOCOPY(Ext2)

public:
  Ext2();
  isize read_file(bu::StringView file, u8 *buffer, u64 offset,
                  usize len) override;
  bu::Vector<bu::Vector<char>> read_dir(bu::StringView path) override;
  u64 file_size(bu::StringView file) override;

private:
  void print_root_dir();
  u32 get_inode_block_group(u32 inode_index);
  u32 find_inode();
  u32 block_index_to_offset(u32 index);
  ext2::Inode read_inode_from_disk(u32 inode_index);

  // FIXME: This should be some kind of reference counted thing for caching
  bu::Vector<u8> read_block_from_disk(u32 block_index);
  bu::Vector<u8> read_inode_block_from_disk(const ext2::Inode &inode,
                                            u32 block_number);
  bu::Vector<u8> read_indirect_block_from_disk(const u32 table_block,
                                               const u32 block_offset);

  u32 find_file_in_directory(ext2::Inode directory, bu::StringView name);
  void for_each_entry_in_dir(
      ext2::Inode directory,
      bu::IterationResult (*func)(const ext2::DirectoryEntry &));

  // FIXME: Optional
  bu::OwnedPtr<ext2::Inode> get_inode_for_file(bu::StringView file);
  ext2::BlockGroupDescriptor read_block_group_entry_from_disk(u32 block_index);
  ext2::DirectoryEntry read_directory_entry_from_disk(u32 block_index);

  bu::OwnedPtr<ext2::SuperBlock> m_superblock;
  u32 m_block_group_count{0};
  u32 m_block_size{0};
  u32 m_block_group_descriptor_table_offset;
};
} // namespace kernel::fs
