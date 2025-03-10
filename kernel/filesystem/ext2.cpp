#include <bustd/math.hpp>
#include <bustd/vector.hpp>
#include <kernel/filesystem/ext2.hpp>
#include <kernel/x86/pata.hpp>
#include <stdio.h>

#define EXT2_SIGNATURE 0xef53
#define EXT2_ROOT_INODE 2

// #define EXT2_DEBUG
#ifdef EXT2_DEBUG
#define DEBUG_PRINTF(...) printf("[ext2] " __VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif

namespace {
constexpr auto block_size_from_log(u32 log) -> u32 { return 1024 << log; }
} // namespace

namespace kernel::filesystem {
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

auto Ext2::read_block_group_entry_from_disk(u32 block_group)
    -> ext2::BlockGroupDescriptor {
  static_assert(sizeof(ext2::BlockGroupDescriptor) == 32);
  DEBUG_PRINTF("block group: %u\n", block_group);
  ext2::BlockGroupDescriptor out{};
  x86::read_bytes_polling(reinterpret_cast<u8 *>(&out),
                          m_block_group_descriptor_table_offset +
                              block_group * sizeof(ext2::BlockGroupDescriptor),
                          sizeof(ext2::BlockGroupDescriptor));
  return out;
}

auto Ext2::read_inode_from_disk(u32 inode_index) -> ext2::Inode {
  ASSERT(inode_index > 0);

  const auto block_group =
      (inode_index - 1) / m_superblock->block_group_inode_count;
  const auto table_index =
      (inode_index - 1) % m_superblock->block_group_inode_count;

  DEBUG_PRINTF("block_group_inode_count: %u\n",
               m_superblock->block_group_inode_count);

  const auto descriptor = read_block_group_entry_from_disk(block_group);
  const auto inode_table = descriptor.inode_table_start_block_addr;

#ifdef EXT2_DEBUG
  DEBUG_PRINTF("Block group: %u, inode_table: %u, table_index: %u\n",
               block_group, inode_table, table_index);
#endif

  static_assert(sizeof(ext2::Inode) == 128);
  ext2::Inode inode;
  x86::read_bytes_polling(reinterpret_cast<u8 *>(&inode),
                          block_index_to_offset(inode_table) +
                              table_index * m_superblock->inode_struct_size,
                          sizeof(inode));
  return inode;
}

auto Ext2::block_index_to_offset(u32 index) -> u32 {
  return m_block_size * index;
}

auto Ext2::get_inode_block_group(u32 inode_index) -> u32 {
  ASSERT(inode_index > 0);
  return (inode_index - 1) / m_superblock->block_group_inode_count;
}

auto Ext2::read_directory_entry_from_disk(u32 offset) -> ext2::DirectoryEntry {
  ext2::DirectoryEntry out;
  x86::read_bytes_polling(reinterpret_cast<u8 *>(&out), offset, sizeof(out));
  return out;
}

void Ext2::print_root_dir() {
  const auto inode = read_inode_from_disk(EXT2_ROOT_INODE);
  ASSERT(inode.is_directory());

  printf("[ext2] Directory listing: ");

  for_each_entry_in_dir(inode, [](const ext2::DirectoryEntry &dir) {
    printf("/");
    for (int i = 0; i < dir.name_len; i++) {
      printf("%c", dir.name[i]);
    }
    printf(" ");
    return bu::IterationResult::Continue;
  });
  puts("");
}

auto Ext2::read_block_from_disk(const u32 block_index) -> bu::Vector<u8> {
  bu::Vector<u8> out(m_block_size);
  out.fill(0, m_block_size);
  x86::read_bytes_polling(out.data(), block_index_to_offset(block_index),
                          m_block_size);
  return out;
}

auto Ext2::read_indirect_block_from_disk(
    const u32 table_block, const u32 block_offset) -> bu::Vector<u8> {
  const auto table = read_block_from_disk(table_block);
  return read_block_from_disk(
      reinterpret_cast<const u32 *>(table.data())[block_offset]);
}

auto Ext2::read_inode_block_from_disk(
    const ext2::Inode &inode, const u32 block_number) -> bu::Vector<u8> {
  const auto direct_block_count = sizeof(ext2::Inode::direct_block_pointers) /
                                  sizeof(ext2::Inode::direct_block_pointers[0]);
  const auto indirect_block_count = m_block_size / sizeof(u32);
  // const auto double_indirect_block_count =
  //     m_block_size * m_block_size / sizeof(u32);
  // const auto triple_indirect_block_count =
  //     m_block_size * m_block_size * m_block_size / sizeof(u32);

  if (block_number < direct_block_count) {
    return read_block_from_disk(inode.direct_block_pointers[block_number]);
  } else if (block_number <= indirect_block_count) {
    return read_indirect_block_from_disk(inode.indir_block_pointer,
                                         block_number - direct_block_count);
  } else {
    TODO("Double and triple indirect blocks not yet implemented");
  }
  return bu::Vector<u8>();
}

auto Ext2::data_from_inode(const u64 inode_index, const u64 offset,
                           const usize bytes, u8 *const buffer) -> isize {
  // FIXME: Should take Optional
  const auto inode = read_inode_from_disk(inode_index);
  // FIXME: Should return an error
  ASSERT(!inode.is_directory());
  const auto file_length = inode.total_size(); // FIXME: Needs to return EOF
  ASSERT(offset + bytes <= file_length);

  const auto start_block = offset / m_block_size;
  const auto end_block = bu::divide_ceil(offset + bytes, (u64)m_block_size) - 1;

  // First block is special and might have to be offset based on the file offset
  // to start reading from.
  auto this_block = read_inode_block_from_disk(inode, start_block);
  const auto first_block_offset = offset % m_block_size;
  memcpy(buffer, this_block.data() + first_block_offset,
         m_block_size - first_block_offset);

  // Whole blocks to read
  auto *buffer_offset = buffer + m_block_size - first_block_offset;
  for (auto i = start_block + 1; i < end_block; i++) {
    this_block = read_inode_block_from_disk(inode, i);
    memcpy(buffer_offset, this_block.data(), m_block_size);
    buffer_offset += m_block_size;
  }

  // Read the final block
  auto bytes_read = static_cast<uintptr_t>(buffer_offset - buffer);
  const auto bytes_left = file_length - bytes_read;
  if (bytes_left > 0) {
    this_block = read_inode_block_from_disk(inode, end_block);
    memcpy(buffer_offset, this_block.data(), bytes_left);
    bytes_read += bytes_left;
  }

  return bytes_read;
}

void Ext2::for_each_entry_in_dir(
    ext2::Inode directory,
    bu::Function<bu::IterationResult(const ext2::DirectoryEntry &)> func) {
  ASSERT(directory.is_directory());

  for (usize i = 0; i < sizeof(directory.direct_block_pointers) / sizeof(u32);
       i++) {
    auto start_offset =
        block_index_to_offset(directory.direct_block_pointers[i]);
    auto this_offset = 0ul;
    if (start_offset == 0) {
      return;
    }
    auto dir = read_directory_entry_from_disk(start_offset + this_offset);

    while (this_offset < m_block_size) {
      if (dir.inode != 0) {
        func(dir);
      }
      this_offset += dir.rec_len;
      dir = read_directory_entry_from_disk(start_offset + this_offset);
    }
  }

  if (directory.indir_block_pointer != 0) {
    TODO("Indir pointers not implemented");
  }
}

// FIXME: This is better solved by implementing a function object in bustd that
// can take lambas with captures
static u32 __find_file_in_dir_found_inode_index = 0;
static const char *__find_file_in_dir_name = nullptr;
static usize __find_file_in_dir_name_len = 0;

auto Ext2::find_file_in_directory(ext2::Inode directory,
                                  bu::StringView name) -> u32 {
  ASSERT(directory.is_directory());
  // ASSERT(name.is_null_terminated());

  __find_file_in_dir_found_inode_index = 0;
  __find_file_in_dir_name = name.data();
  __find_file_in_dir_name_len = name.len();

  // FIXME: Needs a bu::Function
  for_each_entry_in_dir(
      directory, [](const ext2::DirectoryEntry &entry) -> bu::IterationResult {
        if (__find_file_in_dir_name_len == entry.name_len &&
            strncmp(reinterpret_cast<const char *>(entry.name),
                    __find_file_in_dir_name, entry.name_len) == 0) {
          __find_file_in_dir_found_inode_index = entry.inode;
          return bu::IterationResult::Break;
        } else {
          return bu::IterationResult::Continue;
        }
      });
  return __find_file_in_dir_found_inode_index;
}

auto Ext2::get_inode_at_path(bu::StringView file)
    -> bu::Optional<filesystem::Inode> {
  // FIXME: We need some way to handle an error
  ASSERT(file[0] == '/');
  // FIXME: This should be in some path class
  bu::Vector<bu::StringView> segments;
  usize last_index = 0;
  for (usize i = 0; i < file.len(); i++) {
    if (file[i] == '/') {
      segments.push(bu::StringView(file.data() + last_index, i - last_index));
      last_index = i + 1;
    }
  }
  segments.push(
      bu::StringView(file.data() + last_index, file.len() - last_index));

  ext2::Inode inode = read_inode_from_disk(EXT2_ROOT_INODE);
  u32 found_index = 0;
  for (usize i = 1; i < segments.len(); i++) {
    const auto found = find_file_in_directory(inode, segments[i]);
    if (found == 0) {
      return bu::create_none<filesystem::Inode>();
    } else {
      inode = read_inode_from_disk(found);
      found_index = found;
    }
  }
  if (found_index != 0) {
    return bu::create_some<filesystem::Inode>(
        inode.into_system_inode(found_index));
  } else {
    return bu::create_none<filesystem::Inode>();
  }
}

auto Ext2::list_directory(const u64 inode_index)
    -> bu::Optional<bu::Vector<filesystem::DirectoryEntry>> {
  const auto dir = read_inode_from_disk(inode_index);
  ASSERT(dir.is_directory());
  // FIXME: Validation
  // FIXME: use string
  bu::Vector<filesystem::DirectoryEntry> out;
  for_each_entry_in_dir(dir, [&](const ext2::DirectoryEntry &entry) {
    filesystem::DirectoryEntry generic_entry{};
    generic_entry.inode =
        read_inode_from_disk(entry.inode).into_system_inode(entry.inode);
    strncpy(generic_entry.name, entry.name, entry.name_len);
    out.push(generic_entry);
    return bu::IterationResult::Continue;
  });
  return bu::create_some<bu::Vector<filesystem::DirectoryEntry>>(out);
}

} // namespace kernel::filesystem
