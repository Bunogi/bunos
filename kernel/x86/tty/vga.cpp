#include "vga.hpp"
#include <string.h>

constexpr u8 TEXT_WIDTH = 80;
constexpr u8 TEXT_HEIGHT = 25;

namespace kernel::tty::x86 {
Vga::Vga() { clear(); }

u8 Vga::entry_color(Color fg, Color bg) {
  return static_cast<u8>(fg) | static_cast<u8>(bg) << 4;
}

u16 Vga::entry(char c, Color bg) {
  return static_cast<u16>(c) | static_cast<u16>(bg) << 8;
}

void Vga::clear() {
  m_row = m_column = 0;
  m_current_color = Color::White;
  for (u8 y = 0; y < TEXT_HEIGHT; y++) {
    for (u8 x = 0; x < TEXT_WIDTH; x++) {
      const auto index = y * TEXT_WIDTH + x;
      m_termbuffer[index] = entry(' ', Color::White);
    }
  }
}

void Vga::write_char_at(char c, u8 column, u8 row) {
  auto index = row * TEXT_WIDTH + column;
  m_termbuffer[index] = entry(c, m_current_color);
}

void Vga::scroll() {
  for (u8 i = 1; i < TEXT_HEIGHT; i++) {
    auto *previous_line = m_termbuffer + (i - 1) * TEXT_WIDTH;
    const auto *current_line = m_termbuffer + i * TEXT_WIDTH;
    // copy times 2 because the pointer is a u16*, not a u8*
    memcpy(previous_line, current_line, TEXT_WIDTH * 2);
  }
  memset(m_termbuffer + TEXT_WIDTH * (TEXT_HEIGHT - 1), 0, TEXT_WIDTH * 2);
  m_column = 0;
  m_row = TEXT_HEIGHT - 1;
}

void Vga::putchar(char c) {
  if (c == '\n') {
    m_column = 0;
    m_row++;
  } else {
    write_char_at(c, m_column, m_row);
    m_column++;
  }
  if (m_column == TEXT_WIDTH) {
    m_column = 0;
    m_row++;
  }
  if (m_row == TEXT_HEIGHT) {
    scroll();
  }
}

void Vga::write(const char *text, usize len) {
  for (usize i = 0; i < len; i++) {
    putchar(text[i]);
  }
}

} // namespace kernel::tty::x86
