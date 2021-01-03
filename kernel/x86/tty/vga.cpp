#include "vga.hpp"
#include <kernel/x86/io.hpp>
#include <string.h>

constexpr u8 TEXT_WIDTH = 80;
constexpr u8 TEXT_HEIGHT = 25;

namespace kernel::tty::x86 {
Vga::Vga() { clear(); }

Vga::Vga(Vga &&other) {
  m_row = other.m_row;
  m_column = other.m_column;
  m_current_color = other.m_current_color;
}

u8 Vga::entry_color(Color fg, Color bg) {
  return static_cast<u8>(fg) | static_cast<u8>(bg) << 4;
}

u16 Vga::entry(char c, Color bg) {
  return static_cast<u16>(c) | static_cast<u16>(bg) << 8;
}

void Vga::clear() {
  m_row = m_column = 0;
  m_current_color = Color::LightGray;
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
  switch (c) {
  case '\n':
    m_column = 0;
    m_row++;
    break;
  case '\b':
    if (m_column > 0) {
      m_column--;
    }
    break;
  case '\r':
    m_column = 0;
    break;
  default:
    write_char_at(c, m_column, m_row);
    m_column++;
    break;
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
  update_cursor();
}

void Vga::update_cursor() {
  using kernel::x86::io::out_u8;
  u16 pos = m_row * TEXT_WIDTH + m_column;

  // TODO: Name these registers to make things better
  out_u8(0x3D4, 0x0F);
  out_u8(0x3D5, static_cast<u8>(pos & 0xFF));
  out_u8(0x3D4, 0x0E);
  out_u8(0x3D5, static_cast<u8>((pos >> 8) & 0xFF));
}

void Vga::flush() {}

} // namespace kernel::tty::x86
