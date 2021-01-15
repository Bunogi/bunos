#include <kernel/debugsymbols.hpp>
#include <kernel/filesystem/vfs.hpp>
#include <kernel/kprint.hpp>
#include <stdio.h>
#include <stdlib.h>

namespace {
struct SymbolEntry {
  bu::Vector<char> name;
  u32 offset;
};
} // namespace

namespace kernel {

static bu::Vector<SymbolEntry> *s_symbols;
static bool s_symbols_loaded;

void load_debug_symbols() {
  ASSERT_EQ(s_symbols, nullptr);

  s_symbols = new bu::Vector<SymbolEntry>();

  const bu::StringView file = "/boot/kernel.sym";
  printf("Loading debug symbols from %s\n", file.data());

  const auto size = Vfs::instance()->file_size(file);
  bu::Vector<u8> contents(size);
  printf("File size: %u\n", size);
  contents.fill('0', size);

  const auto characters_read =
      Vfs::instance()->read_file(file, contents.data(), 0, size);
  ASSERT_EQ((usize)characters_read, size);

  // Parse the output from nm
  enum class State { ReadingAddress, ReadingSymbol, ReadingType };
  auto state = State::ReadingAddress;

  bu::Vector<char> current_address;
  bu::Vector<char> current_symbol;
  SymbolEntry current_entry{};

  for (isize i = 0; i < characters_read; i++) {
    if (contents[i] == '\n') {
      ASSERT_EQ(state, State::ReadingSymbol);
      current_symbol.push('\0');
      current_entry.name = current_symbol;
      s_symbols->push(current_entry);
      state = State::ReadingAddress;
      current_symbol.clear();
    } else if (contents[i] != ' ') {
      switch (state) {
      case State::ReadingAddress:
        current_address.push(contents[i]);
        break;
      case State::ReadingSymbol:
        current_symbol.push(contents[i]);
        break;
      case State::ReadingType:
        break;
      }
    } else {
      switch (state) {
      case State::ReadingAddress:
        current_address.push('\0');
        current_entry.offset = strtoll(current_address.data(), nullptr, 16);
        state = State::ReadingType;
        current_address.clear();
        break;
      case State::ReadingType:
        state = State::ReadingSymbol;
        break;
      case State::ReadingSymbol:
        current_symbol.push(contents[i]);
        break;
      }
    }
  }

  printf("Loaded %u debug symbols\n", s_symbols->len());

  s_symbols_loaded = true;
}

bu::StringView function_name_from_pc(u32 pc) {
  ASSERT_NE(s_symbols, nullptr);
  ASSERT(s_symbols->len() >= 2);

  auto &symbols = *s_symbols;
  auto *prev_pc = &symbols[0];
  for (usize i = 0; i < s_symbols->len(); i++) {
    if (pc < symbols[i].offset) {
      return bu::StringView(prev_pc->name.data());
    }
    prev_pc = &symbols[i];
  }

  return bu::StringView(prev_pc->name.data());
}

bool debug_symbols_loaded() { return s_symbols_loaded; }

} // namespace kernel
