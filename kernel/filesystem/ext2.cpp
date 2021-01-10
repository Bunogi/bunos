#include <bustd/math.hpp>
#include <bustd/vector.hpp>
#include <kernel/filesystem/ext2.hpp>
#include <kernel/x86/pata.hpp>
#include <stdio.h>

#define EXT2_SIGNATURE 0xef53

#define EXT2_DEBUG
#ifdef EXT2_DEBUG
#define DEBUG_PRINTF(...) printf("[ext2] " __VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif

namespace {
u32 block_size_from_log(u32 log) { return 1024 << log; }
} // namespace

namespace kernel::fs {
Ext2::Ext2() : m_superblock(bu::create_owned<ext2::SuperBlock>()) {
  static_assert(sizeof(ext2::SuperBlock) == 1024);

  const auto bytes_to_read = sizeof(ext2::SuperBlock);
  x86::read_bytes_polling(reinterpret_cast<u8 *>(m_superblock.get()), 1024,
                          bytes_to_read);

  m_block_size = block_size_from_log(m_superblock->block_size_log);

  DEBUG_PRINTF("Inode count: %u, block count: %u, block_size: %u, inode_size: "
               "%u, fragment_size: %u\n",
               m_superblock->inode_count, m_superblock->block_count,
               m_block_size, m_superblock->inode_struct_size,
               block_size_from_log(m_superblock->fragment_size_log));

  DEBUG_PRINTF("Last written: %u, mounted: %u\n", m_superblock->last_write_time,
               m_superblock->last_mount_time);

  DEBUG_PRINTF("Signature: 0x%.4X\n", m_superblock->signature);
  ASSERT_EQ(m_superblock->signature, EXT2_SIGNATURE);

  DEBUG_PRINTF("Ext2 version %u.%u\n", m_superblock->version_major,
               m_superblock->version_minor);
  // FIXME: Is there any reason to support versions < 1.0?
  ASSERT(m_superblock->version_major >= 1);

  m_block_group_count = bu::divide_ceil(m_superblock->block_count,
                                        m_superblock->block_group_block_count);
  ASSERT_EQ(m_block_group_count,
            bu::divide_ceil(m_superblock->inode_count,
                            m_superblock->block_group_inode_count));
  DEBUG_PRINTF("Block groups: %u\n", m_block_group_count);
  m_block_group_descriptor_table_offset = m_block_size == 1024 ? 2048 : 1024;

  print_root_dir();
}

ext2::BlockGroupDescriptor
Ext2::read_block_group_entry_from_disk(u32 block_group) {
  static_assert(sizeof(ext2::BlockGroupDescriptor) == 32);
  DEBUG_PRINTF("block group: %u\n", block_group);
  ext2::BlockGroupDescriptor out{};
  x86::read_bytes_polling(reinterpret_cast<u8 *>(&out),
                          m_block_group_descriptor_table_offset +
                              block_group * sizeof(ext2::BlockGroupDescriptor),
                          sizeof(ext2::BlockGroupDescriptor));
  return out;
}

ext2::Inode Ext2::read_inode_from_disk(u32 inode_index) {
  ASSERT(inode_index > 0);

  /*
  const auto block_index =
      (inode_index - 1) % m_superblock->block_group_inode_count;
  const auto containing_block =
      (block_index * m_superblock->inode_struct_size) / m_block_size;
  */
  const auto block_group =
      (inode_index - 1) / m_superblock->block_group_inode_count;
  const auto table_index =
      (inode_index - 1) % m_superblock->block_group_inode_count;

  DEBUG_PRINTF("block_group_inode_count: %u\n",
               m_superblock->block_group_inode_count);

  const auto descriptor = read_block_group_entry_from_disk(block_group);
  const auto inode_table = descriptor.inode_table_start_block_addr;

#ifdef EXT2_DEBUG
  DEBUG_PRINTF("Block group: %u,", block_group);
  printf(" inode_table: %u,", inode_table);
  printf(" table_index: %u\n", table_index);
#endif

  static_assert(sizeof(ext2::Inode) == 128);
  ext2::Inode inode;
  x86::read_bytes_polling(reinterpret_cast<u8 *>(&inode),
                          block_index_to_offset(inode_table) +
                              table_index * m_superblock->inode_struct_size,
                          sizeof(inode));
  return inode;
}

u32 Ext2::block_index_to_offset(u32 index) { return m_block_size * index; }

u32 Ext2::get_inode_block_group(u32 inode_index) {
  ASSERT(inode_index > 0);
  return (inode_index - 1) / m_superblock->block_group_inode_count;
}

ext2::DirectoryEntry Ext2::read_directory_entry_from_disk(u32 offset) {
  ext2::DirectoryEntry out;
  x86::read_bytes_polling(reinterpret_cast<u8 *>(&out), offset, sizeof(out));
  return out;
}

void Ext2::print_root_dir() {
  const auto inode = read_inode_from_disk(2);
  ASSERT_EQ(inode.type & 0xF000, 0x4000);
  // ASSERT_EQ(inode.size_lower, m_block_size);

  puts("[ext2] Directory listing:");

  for (usize i = 0; i < sizeof(inode.direct_block_pointers) / sizeof(u32);
       i++) {
    auto start_offset = block_index_to_offset(inode.direct_block_pointers[i]);
    auto this_offset = 0ul;
    if (start_offset == 0) {
      break;
    }
    auto dir = read_directory_entry_from_disk(start_offset + this_offset);
    //  while (this_offset < m_block_size) {
    while (this_offset < m_block_size) {
      if (dir.inode != 0) {
        printf("/");
        for (int i = 0; i < dir.name_len; i++) {
          printf("%c", dir.name[i]);
        }
        puts("");
      }
      this_offset += dir.rec_len;
      dir = read_directory_entry_from_disk(start_offset + this_offset);
    }
  }
}

} // namespace kernel::fs
