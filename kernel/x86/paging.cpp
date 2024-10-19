#include <bustd/assert.hpp>
#include <bustd/stddef.hpp>
#include <kernel/x86/paging.hpp>

namespace kernel::x86 {

auto PageDirectoryEntry::as_u32() const -> u32 {
  ASSERT_EQ(page_table_address & 0x00000FFF, 0);
  u32 out = page_table_address & 0xFFFFF000;
  out |= static_cast<u8>(present) << 0;
  out |= static_cast<u8>(read_write) << 1;
  out |= static_cast<u8>(user) << 2;
  out |= static_cast<u8>(write_through) << 3;
  out |= static_cast<u8>(cache_disable) << 4;
  out |= static_cast<u8>(accessed) << 5;
  // out |= static_cast<u8>(zero) << 6; // always zero anyway
  out |= static_cast<u8>(is_large) << 7;
  out |= static_cast<u8>(global) << 8;
  out |= static_cast<u8>(available) << 9;
  return out;
}

auto PageDirectoryEntry::from_u32(u32 from) -> PageDirectoryEntry {
  PageDirectoryEntry out{};
  for (u8 i = 0; i < 9; i++) {
    switch (from & (1 << i)) {
    case 0:
      break;
    case 0x001:
      out.present = true;
      break;
    case 0x002:
      out.read_write = true;
      break;
    case 0x004:
      out.user = true;
      break;
    case 0x008:
      out.write_through = true;
      break;
    case 0x010:
      out.cache_disable = true;
      break;
    case 0x020:
      out.accessed = true;
      break;
    case 0x040:
      out.zero = true;
      break;
    case 0x080:
      out.is_large = true;
      break;
    case 0x100:
      out.global = true;
      break;
    default:
      UNREACHABLE();
    }
  }
  out.available = (from >> 9) & 0x7;
  out.page_table_address = from & 0xFFFFF000;
  return out;
}

auto PageTableEntry::as_u32() const -> u32 {
  ASSERT_EQ(page_address.get() & 0x00000FFF, 0);
  u32 out = (page_address.get() & 0xFFFFF000);
  out |= static_cast<u8>(present) << 0;
  out |= static_cast<u8>(read_write) << 1;
  out |= static_cast<u8>(user) << 2;
  out |= static_cast<u8>(write_through) << 3;
  out |= static_cast<u8>(cache_disable) << 4;
  out |= static_cast<u8>(accessed) << 5;
  // out |= static_cast<u8>(zero) << 6; // always zero anyway
  out |= static_cast<u8>(dirty) << 7;
  out |= static_cast<u8>(global) << 8;
  out |= static_cast<u8>(available) << 9;
  return out;
}

auto PageTableEntry::from_u32(u32 from) -> PageTableEntry {
  PageTableEntry out{};
  for (u8 i = 0; i < 9; i++) {
    switch (from & (1 << i)) {
    case 0:
      break;
    case 0x001:
      out.present = true;
      break;
    case 0x002:
      out.read_write = true;
      break;
    case 0x004:
      out.user = true;
      break;
    case 0x008:
      out.write_through = true;
      break;
    case 0x010:
      out.cache_disable = true;
      break;
    case 0x020:
      out.accessed = true;
      break;
    case 0x040:
      out.dirty = true;
      break;
    case 0x080:
      out.zero = true;
      break;
    case 0x100:
      out.global = true;
      break;
    default:
      UNREACHABLE();
    }
  }
  out.available = (from >> 9) & 0x7;
  out.page_address = kernel::PhysicalAddress(from & 0xFFFFF000);
  return out;
}
} // namespace kernel::x86
