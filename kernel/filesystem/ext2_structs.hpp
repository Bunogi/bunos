#pragma once

#include <bustd/stddef.hpp>
#include <kernel/filesystem/inode.hpp>

namespace kernel::filesystem::ext2 {
struct SuperBlock {
  u32 inode_count;
  u32 block_count;
  u32 superuser_blocks;
  u32 unallocated_blocks;
  u32 unallocated_inodes;
  u32 superblock_block;
  u32 block_size_log;
  u32 fragment_size_log;
  u32 block_group_block_count;
  u32 block_group_fragment_count;
  u32 block_group_inode_count;
  u32 last_mount_time;
  u32 last_write_time;
  u16 mount_count_since_fsck;
  u16 mounts_allowed_before_fcsk;
  u16 signature;
  u16 state;
  u16 error_action;
  u16 version_minor;
  u32 last_consistency_check;
  u32 forced_consistency_check_interval;
  u32 creation_os_id;
  u32 version_major;
  u16 reserved_block_uid;
  u16 reserved_block_gid;
  // EXTENDED FIELDS:
  u32 first_nonreserved_inode;
  u16 inode_struct_size;
  u16 supergroup_block_group;
  u32 optional_features_present;
  u32 required_features_present;
  u32 must_mount_readonly_features;
  struct {
    u64 __padding;
    u64 __padding2;
  } fs_id;
  u8 volume_name[16];
  u8 last_mount_point[64];
  u32 compression_algorithm;
  u8 prealloc_blocks_for_files;
  u8 prealloc_blocks_for_dirs;
  u16 __unused;
  struct {
    u64 __padding;
    u64 __padding2;
  } journal_id;
  u32 journal_inode;
  u32 journal_device;
  u32 orphan_inode_head;
  u8 __unused2[1024 - 236];
};

#define EXT2_I_DIRECTORY 0x4000

struct Inode {
  u16 type;
  u16 uid;
  u32 size_lower;
  u32 last_access_time;
  u32 creation_time;
  u32 modification_time;
  u32 deletion_time;
  u16 gid;
  u16 hard_link_count;
  u32 disk_sectors_in_use;
  u32 flags;
  u32 os_specific_value_1;
  u32 direct_block_pointers[12];
  u32 indir_block_pointer;
  u32 double_indir_block_pointer;
  u32 triple_indir_block_pointer;
  u32 generation_number;
  u32 file_acl;   // Reserved in version 0
  u32 size_upper; // Reserved in version 0
  u32 fragment_block_addr;
  u32 os_specific_value_2[3];

  bool is_directory() const { return (type & EXT2_I_DIRECTORY) != 0; }
  kernel::filesystem::Inode into_system_inode(u64 index) const {
    kernel::filesystem::Inode inode{};
    inode.index = index;
    inode.type =
        type & EXT2_I_DIRECTORY ? InodeType::Directory : InodeType::File;
    inode.file_size = total_size();
    return inode;
  }
  u64 total_size() const {
    return static_cast<u64>(size_upper) << 32 | static_cast<u64>(size_lower);
  }
};

struct BlockGroupDescriptor {
  u32 block_usage_bitmap_block_addr;
  u32 inode_usage_bitmap_block_addr;
  u32 inode_table_start_block_addr;
  u16 unallocated_blocks_count;
  u16 unallocated_inodes_count;
  u16 directories_count;
  u8 __reserved[31 - 18];
};

struct DirectoryEntry {
  u32 inode;
  u16 rec_len;
  u8 name_len;
  u8 file_type;
  char name[256];
};

} // namespace kernel::filesystem::ext2
