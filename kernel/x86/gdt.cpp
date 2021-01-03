#include <bustd/assert.hpp>
#include <bustd/stddef.hpp>
#include <kernel/x86/gdt.hpp>
#include <stdio.h>

extern "C" {
// From gdt.s
void load_gdt(void *start, size_t size);
}

namespace {
struct GDTEntry {
  u32 base;
  u32 limit;
  u8 flags;
};
constexpr int entry_count = 5;
// Change size of this if this changes
static u8 gdt_data[entry_count * 8];

void write_gdt_data(const GDTEntry *const entries, size_t size) {
  for (unsigned i = 0; i < size; i++) {
    GDTEntry source = entries[i];
    volatile auto *target = &gdt_data[i * 8];
    // Check the limit to make sure that it can be encoded
    if ((source.limit > 65536) && ((source.limit & 0xFFF) != 0xFFF)) {
      printf("Tried to encode invalid gdt entry\n");
      printf("Limit: 0x%.8X\n", source.limit);
      UNREACHABLE();
    }
    if (source.limit > 65536) {
      // Adjust granularity if required
      source.limit = source.limit >> 12;
      target[6] = 0xC0;
    } else {
      target[6] = 0x40;
    }

    // Encode the limit
    target[0] = source.limit & 0xFF;
    target[1] = (source.limit >> 8) & 0xFF;
    target[6] |= (source.limit >> 16) & 0xF;

    // Encode the base
    target[2] = source.base & 0xFF;
    target[3] = (source.base >> 8) & 0xFF;
    target[4] = (source.base >> 16) & 0xFF;
    target[7] = (source.base >> 24) & 0xFF;

    // And... flags
    target[5] = source.flags;
  }
}
} // namespace

namespace kernel::x86 {
// Set up flat memory because we use paging anyway
void setup_gdt() {
  GDTEntry gdt[entry_count];
  // Nullentry
  gdt[0].base = 0;
  gdt[0].limit = 0;
  gdt[0].flags = 0;

  // "code"
  gdt[1].base = 0;
  gdt[1].limit = 0xFFFFFFFF;
  gdt[1].flags = 0x9A;

  // "data"
  gdt[2].base = 0;
  gdt[2].limit = 0xFFFFFFFF;
  gdt[2].flags = 0x92;

  // Ring3 code
  gdt[3].base = 0;
  gdt[3].limit = 0xFFFFFFFF;
  gdt[3].flags = 0xFA;

  // Ring3 data
  gdt[4].base = 0;
  gdt[4].limit = 0xFFFFFFFF;
  gdt[4].flags = 0xF2;

  write_gdt_data(gdt, entry_count);

  load_gdt(&gdt_data, entry_count * 8);
  printf("Setup %u gdt entries at %p\n", entry_count, gdt_data);
}
} // namespace kernel::x86
