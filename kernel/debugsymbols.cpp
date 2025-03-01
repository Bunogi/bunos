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
static bu::Vector<SymbolEntry> s_symbols;

void load_debug_symbols() {
  return;
  ASSERT(s_symbols.empty());

  const bu::StringView file = "/boot/kernel.sym";
  printf("Loading debug symbols from %s\n", file.data());

  const auto file_result = Vfs::instance().quick_read_all_data(file);
  const auto contents = *file_result;

  // Parse the output from nm
  enum class State { ReadingAddress, ReadingSymbol, ReadingType };
  auto state = State::ReadingAddress;

  bu::Vector<char> current_address;
  bu::Vector<char> current_symbol;
  SymbolEntry current_entry{};

  for (usize i = 0; i < contents.len(); i++) {
    if (contents[i] == '\n') {
      ASSERT_EQ(state, State::ReadingSymbol);
      current_symbol.push('\0');
      current_entry.name = current_symbol;
      s_symbols.push(current_entry);
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

  printf("Loaded %u debug symbols\n", s_symbols.len());
}

auto function_name_from_pc(usize pc) -> const char * {
  if (s_symbols.empty()) {
    return "<missing debug symbols>";
  }
  const char *name = "<unknown>";
  for (usize i = 0; i < s_symbols.len(); i++) {
    if (pc < s_symbols[i].offset) {
      break;
    }
    name = s_symbols[i].name.data();
  }

  return name;
}

} // namespace kernel
