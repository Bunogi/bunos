#pragma once

#include <bustd/function.hpp>
#include <bustd/iteration.hpp>
#include <bustd/macros.hpp>
#include <bustd/ownedptr.hpp>
#include <bustd/stddef.hpp>
#include <bustd/stringview.hpp>
#include <bustd/vector.hpp>
#include <kernel/filesystem/ext2_structs.hpp>
#include <kernel/filesystem/ifilesystem.hpp>

namespace kernel::filesystem {
class Ext2 : public IFileSystem {
  BU_NOCOPY(Ext2)

public:
  Ext2();
  auto get_inode_at_path(bu::StringView path)
      -> bu::Optional<filesystem::Inode> override;
  auto data_from_inode(u64 inode_index, u64 offset, usize bytes,
                       u8 *buffer) -> isize override;
  auto list_directory(u64 inode_index)
      -> bu::Optional<bu::Vector<filesystem::DirectoryEntry>> override;

private:
  void print_root_dir();
  auto get_inode_block_group(u32 inode_index) -> u32;
  auto find_inode() -> u32;
  auto block_index_to_offset(u32 index) -> u32;
  auto read_inode_from_disk(u32 inode_index) -> ext2::Inode;

  // FIXME: This should be some kind of reference counted thing for caching
  auto read_block_from_disk(u32 block_index) -> bu::Vector<u8>;
  auto read_inode_block_from_disk(const ext2::Inode &inode,
                                  u32 block_number) -> bu::Vector<u8>;
  auto read_indirect_block_from_disk(const u32 table_block,
                                     const u32 block_offset) -> bu::Vector<u8>;

  auto find_file_in_directory(ext2::Inode directory,
                              bu::StringView name) -> u32;
  void for_each_entry_in_dir(
      ext2::Inode directory,
      bu::Function<bu::IterationResult(const ext2::DirectoryEntry &)>);

  // FIXME: Optional
  auto get_inode_for_file(bu::StringView file) -> bu::OwnedPtr<ext2::Inode>;
  auto read_block_group_entry_from_disk(u32 block_index)
      -> ext2::BlockGroupDescriptor;
  auto read_directory_entry_from_disk(u32 block_index) -> ext2::DirectoryEntry;

  bu::OwnedPtr<ext2::SuperBlock> m_superblock;
  u32 m_block_group_count{0};
  u32 m_block_size{0};
  u32 m_block_group_descriptor_table_offset;
};
} // namespace kernel::filesystem
