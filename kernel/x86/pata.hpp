#pragma once

#include <bustd/stddef.hpp>

namespace kernel::x86 {

void initialize_pata();
auto read_sectors_polling(u8 *buffer, u64 sector, u8 sector_count) -> usize;
auto read_bytes_polling(u8 *buffer, u64 offset, usize len) -> usize;
auto disk_sector_count() -> u64;
auto disk_sector_size_in_bytes() -> u16;
} // namespace kernel::x86
