#pragma once

#include <bustd/stddef.hpp>

namespace kernel::x86 {

void initialize_pata();
usize read_sectors_polling(u8 *buffer, u64 sector, u8 sector_count);
usize read_bytes_polling(u8 *buffer, u64 offset, usize len);
u64 disk_sector_count();
u16 disk_sector_size_in_bytes();
} // namespace kernel::x86
