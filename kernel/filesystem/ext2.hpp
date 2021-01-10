#pragma once

#include <bustd/macros.hpp>
#include <bustd/ownedptr.hpp>
#include <bustd/stddef.hpp>
#include <kernel/filesystem/ext2_structs.hpp>

namespace kernel::fs {
class Ext2 {
  BU_NOCOPY(Ext2)

public:
  Ext2();

private:
  void print_root_dir();
  u32 get_inode_block_group(u32 inode_index);
  u32 find_inode();
  u32 block_index_to_offset(u32 index);
  ext2::Inode read_inode_from_disk(u32 inode_index);
  ext2::BlockGroupDescriptor read_block_group_entry_from_disk(u32 block_index);
  ext2::DirectoryEntry read_directory_entry_from_disk(u32 block_index);

  bu::OwnedPtr<ext2::SuperBlock> m_superblock;
  u32 m_block_group_count{0};
  u32 m_block_size{0};
  u32 m_block_group_descriptor_table_offset;
};
} // namespace kernel::fs
