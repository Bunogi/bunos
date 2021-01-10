#include <bustd/math.hpp>
#include <bustd/stddef.hpp>
#include <bustd/stringview.hpp>
#include <bustd/vector.hpp>
#include <kernel/x86/io.hpp>
#include <kernel/x86/pata.hpp>

//#define PATA_DEBUG
#ifdef PATA_DEBUG
#include <stdio.h>
#define DEBUG_PRINTF(...) printf("[pata] " __VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif

namespace {
// Primary bus only
namespace registers {
constexpr u16 io_base = 0x1f0;
constexpr u16 data = io_base + 0;
constexpr u16 error = io_base + 1;    // read only
constexpr u16 features = io_base + 1; // write only
constexpr u16 sector_count = io_base + 2;
constexpr u16 lba_low = io_base + 3;
constexpr u16 lba_mid = io_base + 4;
constexpr u16 lba_high = io_base + 5;
constexpr u16 drive_head = io_base + 6;
constexpr u16 status = io_base + 7;  // read only
constexpr u16 command = io_base + 7; // write only

constexpr u16 control_base = 0x3F6;
constexpr u16 status_alt = control_base + 0; // read only
constexpr u16 control = control_base + 0;    // write only
constexpr u16 address = control_base + 1;

} // namespace registers

enum class Command : u8 {
  SoftwareReset = 0x04,
  ReadSectors = 0x20,
  Identify = 0xEC
};

enum class Drive : u8 { Master = 0xA0, Slave = 0xB0 };

enum class Status : u8 { Busy = 0x80, Drq = 0x08, Err = 0x01 };

using namespace kernel::x86;

#ifdef PATA_DEBUG
bu::StringView drive_name(Drive d) {
  if (d == Drive::Master) {
    return "Master";
  } else {
    return "Slave";
  }
}
#endif

void reset_bus() {
  out_u8(registers::control, static_cast<u8>(Command::SoftwareReset));
  in_u8(registers::status_alt);
  in_u8(registers::status_alt);
  in_u8(registers::status_alt);
  in_u8(registers::status_alt);
  out_u8(registers::control, 0);
  in_u8(registers::status_alt);
  in_u8(registers::status_alt);
  in_u8(registers::status_alt);
  in_u8(registers::status_alt);
}

void select_drive(Drive d) {
  out_u8(registers::drive_head, static_cast<u8>(d));
  in_u8(registers::status_alt);
  in_u8(registers::status_alt);
  in_u8(registers::status_alt);
  in_u8(registers::status_alt);
}

bool disk_present() {
  out_u8(registers::sector_count, 0);
  out_u8(registers::lba_low, 0);
  out_u8(registers::lba_mid, 0);
  out_u8(registers::lba_high, 0);

  out_u8(registers::command, (u8)Command::Identify);

  if (in_u8(registers::status_alt) == 0) {
    return false;
  } else {
    return true;
  }
}

// The following did not work in qemu and i don't know why but whatever
/*
bu::Vector<u16> identify_atapi(Drive d) {
  select_drive(d);
  const u8 count = in_u8(registers::sector_count);
  const u8 lba_low = in_u8(registers::lba_low);
  const u8 lba_mid = in_u8(registers::lba_mid);
  const u8 lba_high = in_u8(registers::lba_high);

  DEBUG_PRINTF("Count: 0x%.2x, lba_low 0x%.2x, lba_mid: 0x%.2x, lba_high:
0x%.2x\n", count, lba_low, lba_mid, lba_high);

  if (count == 1 && lba_low == 0x1 && lba_mid == 0x0 && lba_high == 0x0) {
    DEBUG_PRINTF("Not a packet device :(\n");
  } else if (count == 1 && lba_low == 1 && lba_mid == 0x14 &&
             lba_high == 0xeb) {
    puts("YEAH");
  } else {
    puts("NAH");
  }
  return bu::Vector<u16>();
}
*/

bu::Vector<u16> identify(Drive d) {
  select_drive(d);

  const auto present = disk_present();
  if (!present) {
    DEBUG_PRINTF("[pata] %s drive not connected\n", drive_name(d));
    return bu::Vector<u16>();
  }

  while ((in_u8(registers::status_alt) & (u8)Status::Busy) != 0) {
  }

  // Some drives apparently don't support ata or something so just give up
  if (in_u8(registers::lba_high) != 0 || in_u8(registers::lba_mid) != 0) {
    // FIXME: Don't panic :)
    KERNEL_PANIC("Non-standard drive detected");
  }

  u8 status;
  while (((status = in_u8(registers::status_alt)) &
          ((u8)Status::Err | (u8)Status::Drq)) == 0) {
  }

  if ((status & (u8)Status::Err) != 0) {
    KERNEL_PANIC("Drive error");
  } else if ((status & (u8)Status::Drq) == 0) {
    KERNEL_PANIC("Drq not set?");
  }

  bu::Vector<u16> out(256);
  for (int i = 0; i < 256; i++) {
    out.push(in_u16(registers::data));
  }
  DEBUG_PRINTF("[pata] %s drive connected!\n", drive_name(d));
  return out;
}

struct DriveData {
  u32 lba28_sector_count;
  bool supports_lba48;
  u64 lba48_sector_count;
  bool must_use_lba48;
  u8 max_multiple_rw_count;
  u32 sector_size_in_bytes;
};

DriveData parse_info(bu::Vector<u16> &&identify_data) {
  DriveData out{};
  out.supports_lba48 = (identify_data[83] & 0x0400) != 0;
  DEBUG_PRINTF("data[60]: 0x%.4X data[61]: 0x%.4X\n", identify_data[60],
               identify_data[61]);
  out.lba28_sector_count = *reinterpret_cast<u32 *>(&identify_data.data()[60]);
  out.lba48_sector_count = *reinterpret_cast<u64 *>(&identify_data.data()[100]);
  DEBUG_PRINTF("Sector count(lba28): %u, lba48_support? %s(%u sectors)\n",
               out.lba28_sector_count, out.supports_lba48 ? "yes" : "no",
               out.lba48_sector_count);

  out.max_multiple_rw_count = identify_data[47] & 0xFF;
  DEBUG_PRINTF("Max multiwrite/read size: %u\n", out.max_multiple_rw_count);

  u16 sector_size = identify_data[106];
  if ((sector_size & 0x4000) == 0 && (sector_size & ~0x8000) == 0) {
    // Sector size info is valid
    TODO("Read physical/logical sector size");
  } else {
    out.sector_size_in_bytes = 512;
  }
  DEBUG_PRINTF("Logical sector size: %u\n", out.sector_size_in_bytes);
  // out.
  return out;
}

DriveData s_master_drive;

} // namespace

namespace kernel::x86 {

void initialize_pata() {
  reset_bus();

  auto master_data = identify(Drive::Master);
  if (!master_data.is_empty()) {
    s_master_drive = parse_info(bu::move(master_data));
  }

  auto slave_data = identify(Drive::Slave);
  if (!slave_data.is_empty()) {
    parse_info(bu::move(slave_data));
  }

  // FIXME: We should probably be smarter about this
  select_drive(Drive::Master);
}

usize read_sectors_polling(u8 *buffer, u64 sector, u8 sector_count) {
  ASSERT(sector_count <= s_master_drive.lba28_sector_count);
  ASSERT(sector + sector_count <= s_master_drive.lba28_sector_count);
  if (sector_count == 0) {
    ASSERT(sector + s_master_drive.lba28_sector_count >= 256);
  }
  ASSERT(sector <= 0xFFFFFF);

  // FIXME: Maybe support more than one drive
  out_u8(registers::drive_head, 0xE0 | ((sector >> 24) & 0xF));
  out_u8(registers::sector_count, sector_count);
  out_u8(registers::lba_low, sector & 0xFF);
  out_u8(registers::lba_mid, (sector >> 8) & 0xFF);
  out_u8(registers::lba_high, (sector >> 16) & 0xFF);
  out_u8(registers::command, (u8)Command::ReadSectors);

  for (int i = 0; i < sector_count; i++) {
    // Poll
    u8 status;
    while (((status = in_u8(registers::status_alt)) & (u8)Status::Busy) != 0) {
    }

    while (((status = in_u8(registers::status_alt)) &
            ((u8)Status::Drq | (u8)Status::Err)) == 0) {
    }

    if ((status & (u8)Status::Err) != 0) {
      KERNEL_PANIC("Got error from drive :(");
    }

    in_u16_string(registers::data,
                  reinterpret_cast<u16 *>(
                      buffer + i * s_master_drive.sector_size_in_bytes),
                  s_master_drive.sector_size_in_bytes / 2);
  }

  ASSERT_EQ(in_u8(registers::status_alt) & ((u8)Status::Drq | (u8)Status::Err),
            0);

  return 0;
}

usize read_bytes_polling(u8 *buffer, u64 offset, usize len) {
  auto sector_count =
      bu::max(len / s_master_drive.sector_size_in_bytes, usize(1));
  const auto overflow = len % s_master_drive.sector_size_in_bytes;
  if (overflow > 0) {
    sector_count++;
  }
  const auto sector_offset = offset / s_master_drive.sector_size_in_bytes;
  const auto rest_of_offset = offset % s_master_drive.sector_size_in_bytes;
  // FIXME: This is a workaround for a bug in printf where it doesn't like many
  // parameters
#ifdef PATA_DEBUG
  DEBUG_PRINTF("sector_size: %u, ", s_master_drive.sector_size_in_bytes);
  printf("len: %u, ", len);
  printf("sector_offset: %u, ", sector_offset);
  printf("sectors_to_read: %u\n", sector_count);
#endif

  // FIXME: This could probably be more efficient
  // Avoid writing outside the bounds of buffer.
  bu::Vector<u8> intermediate_buffer(sector_count *
                                     s_master_drive.sector_size_in_bytes);
  read_sectors_polling(intermediate_buffer.data(), sector_offset, sector_count);
  memcpy(buffer, intermediate_buffer.data() + rest_of_offset, len);
  return len - overflow;
}

u16 disk_sector_size_in_bytes() { return s_master_drive.sector_size_in_bytes; }

u64 disk_sector_count() { return s_master_drive.lba28_sector_count; }

} // namespace kernel::x86
