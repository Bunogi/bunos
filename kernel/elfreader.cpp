#include <bustd/math.hpp>
#include <bustd/vector.hpp>
#include <kernel/elfreader.hpp>
#include <kernel/filesystem/vfs.hpp>
#include <kernel/memory.hpp>
#include <kernel/process.hpp>
#include <kernel/x86/memory.hpp>
#include <string.h>

#define ELFSIZE 32
#include <kernel/exec_elf.h>

#define DEBUG_ELF
#ifdef DEBUG_ELF
#include <stdio.h>
#define ELFREADER_PREFIX "[ELFreader] "
#define DEBUG_PRINTF(...) printf(ELFREADER_PREFIX __VA_ARGS__)
#define DEBUG_PUTS(...) puts(ELFREADER_PREFIX __VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#define DEBUG_PUTS(...)
#endif

namespace {
bool validate_ident(const u8 *data) {
  if (data[EI_MAG0] != ELFMAG0 || data[EI_MAG1] != ELFMAG1 ||
      data[EI_MAG2] != ELFMAG2 || data[EI_MAG3] != ELFMAG3) {
    DEBUG_PUTS("Invalid magic number");
    return false;
  }

  if (data[EI_CLASS] != ELFCLASS32) {
    DEBUG_PUTS("Invalid class");
    return false;
  }

  if (data[EI_DATA] != ELFDATA2LSB) {
    DEBUG_PUTS("Invalid data encoding");
    return false;
  }

  if (data[EI_VERSION] != EV_CURRENT) {
    DEBUG_PUTS("Invalid ELF version");
    return false;
  }

  // EI_OSABI seems to be an openbsd thing, so ignore it
  return true;
}

bool validate_header(const Elf32_Ehdr *header) {
  if (!validate_ident(header->e_ident)) {
    return false;
  }

  if (header->e_type != ET_EXEC) {
    DEBUG_PUTS("error: Invalid type");
    return false;
  }

  if (header->e_machine != EM_386) {
    DEBUG_PUTS("error: Invalid machine");
    return false;
  }

  if (header->e_version != EV_CURRENT) {
    DEBUG_PUTS("error: Invalid version");
    return false;
  }

  if (header->e_entry == 0) {
    DEBUG_PUTS("error: Entry is zero");
    return false;
  }

  return true;
}

bool allocate_program_headers(const u8 *const data, const usize data_len,
                              kernel::Process &process,
                              const Elf32_Ehdr *header) {
  const auto header_offset = header->e_phoff;
  if (header_offset == 0) {
    DEBUG_PUTS("error: No program header found");
    return false;
  }
  const auto header_count = header->e_phnum;
  const auto header_size = header->e_phentsize;

  for (usize i = 0; i < header_count; i++) {
    const auto *prog_header = reinterpret_cast<const Elf32_Phdr *>(
        &data[header_offset + i * header_size]);

    switch (prog_header->p_type) {
    case PT_NULL:
    case PT_NOTE:
      // Can safely ignore these
      DEBUG_PRINTF("Ignored program header of type %u", prog_header->p_type);
      continue;
    case PT_LOAD:
      break;
    case PT_DYNAMIC:
      DEBUG_PUTS("error: found dynamic section");
      return false;
    case PT_INTERP:
      DEBUG_PUTS("error: Program has interpreter");
      return false;
    case PT_SHLIB:
    default:
      DEBUG_PRINTF("Unrecognized field type %u\n", prog_header->p_type);
      return false;
    }

    // Only support PT_LOAD for now
    ASSERT_EQ(prog_header->p_type, PT_LOAD);

    DEBUG_PRINTF("p_align: %x\n", prog_header->p_align);
    ASSERT(prog_header->p_offset + prog_header->p_memsz < data_len);
    // FIXME: Is this right?
    ASSERT_EQ(prog_header->p_align, 4096);

    auto *segment_offset = data + prog_header->p_offset;
    auto vaddr = kernel::VirtualAddress(prog_header->p_vaddr);
    auto pages_to_allocate =
        bu::divide_ceil(prog_header->p_memsz, static_cast<Elf32_Word>(4096));
    DEBUG_PRINTF("memsz: %x, filesz: %x, have to allocate %u pages\n",
                 prog_header->p_memsz, prog_header->p_filesz,
                 pages_to_allocate);
    DEBUG_PRINTF("p_virtaddr: %p\n", prog_header->p_vaddr);

    // Copy almost all
    for (usize j = 0; j < pages_to_allocate - 1; j++) {
      ASSERT_EQ(vaddr.get() & 0xFFF, 0);
      ASSERT(kernel::x86::map_user_memory(process, vaddr));
      if (!(prog_header->p_flags & SHF_WRITE)) {
        kernel::x86::set_user_mem_no_write(process, vaddr);
      }
      memcpy(vaddr.ptr(), segment_offset, 4096);
      segment_offset += 4096;
      vaddr += 4096;
    }

    // Copy the rest
    ASSERT(kernel::x86::map_user_memory(process, vaddr));
    const auto remainder = prog_header->p_filesz % 4096;
    memcpy(vaddr.ptr(), segment_offset, remainder);
    segment_offset += remainder;

    ASSERT(prog_header->p_memsz >= prog_header->p_filesz);
    const auto to_zero = prog_header->p_memsz - prog_header->p_filesz;
    ASSERT(to_zero + remainder <= 4096);
    memset(reinterpret_cast<void *>(vaddr.get() + remainder), '0', to_zero);
    if (!(prog_header->p_flags & SHF_WRITE)) {
      kernel::x86::set_user_mem_no_write(process, vaddr);
    }
  }
  return true;
}
} // namespace

namespace kernel::elf {
void (*parse(Process &proc, bu::StringView file))() {
  const auto file_size = Vfs::instance()->file_size(file);
  bu::Vector<u8> data_buf(file_size);
  data_buf.fill('\0', file_size);

  _x86_set_page_directory(proc.page_dir().get());

  // FIXME: We should not read the entire file in one swoop like this
  const auto characters_read =
      Vfs::instance()->read_file(file, data_buf.data(), 0, file_size);
  ASSERT_EQ((usize)characters_read, file_size);
  DEBUG_PRINTF("Read %u chars\n", characters_read);

  const auto *data = data_buf.data();
  const auto *header = reinterpret_cast<const Elf32_Ehdr *>(data);

  if (!validate_header(header)) {
    return nullptr;
  }

  DEBUG_PUTS("are HERE");
  if (!allocate_program_headers(data, file_size, proc, header)) {
    return nullptr;
  }
  DEBUG_PUTS("AM HERE");

  // TODO();

  return reinterpret_cast<void (*)()>(header->e_entry);
}
} // namespace kernel::elf
