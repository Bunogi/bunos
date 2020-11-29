#pragma once

#include <bustd/stddef.hpp>
#include <kernel/tty/ittydevice.hpp>

// TODO: Make this a generic interface and stuff when global constructors are
// set up

namespace kernel::tty::x86 {
class Vga : public tty::IDevice {
public:
  Vga();

  virtual void write(const char *text, usize len) override;
  virtual void putchar(char c) override;

  enum class Color : u8 {
    Black = 0,
    Blue = 1,
    Green = 2,
    Cyan = 3,
    Red = 4,
    Magenta = 5,
    Brown = 6,
    LightGray = 7,
    DarkGray = 8,
    LightBlue = 9,
    LightGreen = 10,
    LightCyan = 11,
    LightRed = 12,
    LightMagenta = 13,
    LightBrown = 14,
    White = 15,
    MAX
  };
  void write_char_at(char c, u8 column, u8 row);

private:
  void scroll();
  void clear();
  u16 ascii_to_vga(char c);

  u8 entry_color(Color fg, Color bg);
  u16 entry(char c, Color bg);
  u8 m_row{0};
  u8 m_column{0};
  Color m_current_color{Color::White};

  u16 *m_termbuffer{(u16 *)0xB8000};
};
} // namespace kernel::tty::x86