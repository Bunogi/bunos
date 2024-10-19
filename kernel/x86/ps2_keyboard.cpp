#include <ctype.h>
#include <kernel/interrupts.hpp>
#include <kernel/process/keyboardfile.hpp>
#include <kernel/x86/interrupts.hpp>
#include <kernel/x86/ps2.hpp>
#include <kernel/x86/ps2_keyboard.hpp>
#include <kernel/x86/ps2_keyboard_scancodes.hpp>
#include <stdio.h>

namespace {
static kernel::x86::PS2Keyboard *volatile s_keyboard;

constexpr u8 keyboard_vector = 0x20 + 0x1;
auto interrupt_handler(kernel::x86::InterruptFrame *) -> bool {
  s_keyboard->handle_message(kernel::x86::ps2::read_isr_response());
  return true;
}

namespace cmd {
constexpr u8 enable_scanning = 0xF4;
constexpr u8 set_scancode_set = 0xF0;

constexpr u8 max_data_byte_count = 1;
} // namespace cmd
namespace resp {
constexpr u8 ack = 0xFA;
constexpr u8 resend = 0xFE;
} // namespace resp

constexpr auto get_data_byte_count(const u8 command) -> u8 {
  switch (command) {
  case cmd::set_scancode_set:
    return 1;
  default:
    return 0;
  }
}
} // namespace

namespace kernel::x86 {

PS2Keyboard::PS2Keyboard(PS2Device dev, PS2DeviceType type)
    : m_device(dev), m_type(type), m_state(State::ExpectingAck) {
  ASSERT_EQ(s_keyboard, nullptr);
  s_keyboard = this;
  // Translation(Scan code set 1) not supported yet
  ASSERT_EQ(m_type, PS2DeviceType::Keyboard);

  interrupts::register_handler(keyboard_vector, &interrupt_handler);

  // Fuck scancode set 2
  m_command_queue.push(cmd::set_scancode_set);
  m_command_queue.push(3);
  m_scancode_set = 3;

  m_command_queue.push(cmd::enable_scanning);

  send_command();
}

void PS2Keyboard::send_command() {
  if (!m_command_queue.is_empty()) {
    const auto command = m_command_queue.head();
    ps2::write(m_device, command);
    const auto num_data_bytes = get_data_byte_count(command);
    if (num_data_bytes > 0) {
      // Offset by 1 to avoid reading the command again
      u8 buffer[cmd::max_data_byte_count + 1];
      const auto read = m_command_queue.read(buffer, num_data_bytes + 1);
      ASSERT_EQ(read, static_cast<usize>(num_data_bytes + 1));

      for (int i = 1; i <= num_data_bytes; i++) {
        ps2::write(m_device, buffer[i]);
      }
    }
  }
}

void PS2Keyboard::handle_message(u8 message) {
  // S1/2 not supported
  ASSERT_EQ(m_scancode_set, 3);
  switch (m_state) {
  case State::ExpectingAck:
    ASSERT(message == resp::resend || message == resp::ack);
    if (message == resp::resend) {
      send_command();
    } else if (message == resp::ack) {
      const auto prev_command = m_command_queue.pop();
      const auto prev_command_data_bytes = get_data_byte_count(prev_command);
      if (prev_command_data_bytes > 0) {
        m_command_queue.drop(prev_command_data_bytes);
      }

      if (m_command_queue.is_empty()) {
        m_state = State::ExpectingKeyCode;
      } else {
        send_command();
      }
    } else {
      // unknown keycode :(
    }
    return;
  default:
    feed_scancode(message);
    return;
  }
}

void PS2Keyboard::feed_scancode(u8 code) {
  if (m_state == State::ExpectingKeyCode && code == S3_RELEASE_PREFIX) {
    m_state = State::Relase;
    return;
  }
  if (code == S3_LSHIFT || code == S3_RSHIFT) {
    if (m_state == State::ExpectingKeyCode) {
      m_shift_count++;
    } else {
      m_shift_count--;
      m_state = State::ExpectingKeyCode;
    }
    return;
  }

  u8 key_ascii = scancode_to_ascii(code);
  if (key_ascii != '\0') {
    if (m_state == State::ExpectingKeyCode) {
      if (m_shift_count > 0) {
        key_ascii = with_shift(key_ascii);
      }
      auto &keyboard_listener = process::KeyboardFile::instance();
      keyboard_listener.key_trigger(key_ascii);
    } else {
      m_state = State::ExpectingKeyCode;
    }
  }
}

// clang-format off
auto PS2Keyboard::scancode_to_ascii(u8 code) -> u8 {
  switch (code) {
  case S3_0: case S3_NUM0: return '0';
  case S3_1: case S3_NUM1: return '1';
  case S3_2: case S3_NUM2: return '2';
  case S3_3: case S3_NUM3: return '3';
  case S3_4: case S3_NUM4: return '4';
  case S3_5: case S3_NUM5: return '5';
  case S3_6: case S3_NUM6: return '6';
  case S3_7: case S3_NUM7: return '7';
  case S3_8: case S3_NUM8: return '8';
  case S3_9: case S3_NUM9: return '9';
  case S3_A: return 'a';
  case S3_B: return 'b';
  case S3_C: return 'c';
  case S3_D: return 'd';
  case S3_E: return 'e';
  case S3_F: return 'f'; case S3_G: return 'g';
  case S3_H: return 'h';
  case S3_I: return 'i';
  case S3_J: return 'j';
  case S3_K: return 'k';
  case S3_L: return 'l';
  case S3_M: return 'm';
  case S3_N: return 'n';
  case S3_O: return 'o';
  case S3_P: return 'p';
  case S3_Q: return 'q';
  case S3_R: return 'r';
  case S3_S: return 's';
  case S3_T: return 't';
  case S3_U: return 'u';
  case S3_V: return 'v';
  case S3_W: return 'w';
  case S3_X: return 'x';
  case S3_Y: return 'y';
  case S3_Z: return 'z';
  case S3_BACKTICK: return '`';
  case S3_MINUS: return '-';
  case S3_BACKSLASH: return '\\';
  case S3_BACKSPACE: return '\b';
  case S3_SPACE: return ' ';
  case S3_ENTER: case S3_NUMENTER: return '\n';
  case S3_NUMPLUS: return '+';
  case S3_EQUALS: return '=';
  case S3_TAB: return '\t';
  case S3_SEMICOLON: return ';';
  case S3_COMMA: return ',';
  case S3_PERIOD: return '.';
  case S3_QUOTE: return '\'';
  case S3_SLASH: return '/';
  case S3_DELETE: return 0x7F;
  case S3_LSHIFT: case S3_LCTRL: case S3_LSUPER: case S3_LALT: case S3_RSHIFT: case S3_RCTRL:
  case S3_RSUPER: case S3_RALT: case S3_APPS: case S3_CAPS: case S3_F1: case S3_F2: case S3_F3: case S3_F4:
  case S3_F5: case S3_F6: case S3_F7: case S3_F8: case S3_F9: case S3_F10: case S3_F11: case S3_F12: case S3_PRINT:
  case S3_SCROLLOCK: case S3_PAUSE: case S3_INSERT: case S3_HOME: case S3_PGUP: case S3_PGDN:
  case S3_NUMLCK: case S3_END: case S3_LEFT: case S3_RIGHT: case S3_UP: case S3_DOWN:
  // FIXME: Do we need this one?
  case S3_RELEASE_PREFIX:
    return '\0';
  default:
    printf("PS2 Keyboard: unhandled scancode 0x%.2X\n", code);
    return '\0';
  }
}

auto PS2Keyboard::with_shift(u8 ascii) -> u8 {
  if (isalpha(ascii)) {
    return toupper(ascii);
  }
  switch (ascii) {
  case '1': return '!';
  case '2': return '@';
  case '3': return '#';
  case '4': return '$';
  case '5': return '%';
  case '6': return '^';
  case '7': return '&';
  case '8': return '*';
  case '9': return '(';
  case '0': return ')';
  case ';': return ':';
  case '\'': return '"';
  case '\\': return '|';
  case '/': return '?';
  case ',': return '<';
  case '.': return '>';
  case '-': return '_';
  case '=': return '+';
  case '`': return '~';
  default:
    return ascii;
  }
}
// clang-format on
} // namespace kernel::x86
