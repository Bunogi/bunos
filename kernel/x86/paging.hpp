#pragma once

#include <bustd/stddef.hpp>
#pragma once
#pragma once
#include <kernel/memory.hpp>

namespace kernel::x86 {
class PageTableEntry {
public:
  bool present : 1;
  bool read_write : 1;
  bool user : 1;
  bool write_through : 1;
  bool cache_disable : 1;
  bool accessed : 1;
  bool dirty : 1;
  bool zero : 1; // must be zero in a valid entry
  bool global : 1;
  u8 available : 3;
  PhysicalAddress page_address;

  [[nodiscard]] auto as_u32() const -> u32;

  static auto from_u32(u32 from) -> PageTableEntry;
};

class PageDirectoryEntry {
public:
  bool present : 1;
  bool read_write : 1;
  bool user : 1;
  bool write_through : 1;
  bool cache_disable : 1;
  bool accessed : 1;
  bool zero : 1; // must be zero in a valid entry
  bool is_large : 1;
  bool global : 1;
  u8 available : 3;
  u32 page_table_address;

  [[nodiscard]] auto as_u32() const -> u32;

  static auto from_u32(u32 from) -> PageDirectoryEntry;
};
} // namespace kernel::x86
