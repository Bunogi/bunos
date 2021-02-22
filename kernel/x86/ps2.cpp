#include <bustd/assert.hpp>
#include <kernel/x86/io.hpp>
#include <kernel/x86/pic.hpp>
#include <kernel/x86/ps2.hpp>
#include <kernel/x86/ps2_keyboard.hpp>
#include <stdio.h>

using namespace kernel::x86;

namespace {
constexpr u16 data_port = 0x60;
constexpr u16 status_register = 0x64;  // read only
constexpr u16 command_register = 0x64; // write only
namespace status {
constexpr u8 output_buffer_full = 0x01;
constexpr u8 input_buffer_full = 0x02;
constexpr u8 system_flag = 0x04;
constexpr u8 command_for_controller = 0x08;
constexpr u8 timeout = 0x40;
constexpr u8 parity_error = 0x80;
} // namespace status

namespace cmd {
constexpr u8 disable_channel0 = 0xAD;
constexpr u8 disable_channel1 = 0xA7;
constexpr u8 read_configuration = 0x20;
constexpr u8 write_configuration = 0x60;
constexpr u8 self_test = 0xAA;
constexpr u8 test_channel0 = 0xAB;
constexpr u8 test_channel1 = 0xA9;
constexpr u8 write_to_2nd_device = 0xD4;
constexpr u8 enable_channel0 = 0xAE;
constexpr u8 enable_channel1 = 0xA8;
} // namespace cmd

namespace dev_cmd {
constexpr u8 disable_scanning = 0xF5;
constexpr u8 ack = 0xFA;
constexpr u8 reset = 0xFF;
constexpr u8 identify = 0xF2;
} // namespace dev_cmd

void write_command(u8 command, u8 data) {
  out_u8(command_register, command);
  while (in_u8(status_register & status::output_buffer_full) != 0) {
  }
  out_u8(data_port, data);
}

u8 read_response() {
  while (in_u8(status_register & status::output_buffer_full) != 0) {
  }
  return in_u8(data_port);
}

u8 read_input_response() {
  while (in_u8(status_register & status::input_buffer_full) != 0) {
  }
  return in_u8(data_port);
}

u8 read_command(u8 command) {
  out_u8(command_register, command);
  return read_response();
}

bool detect_device_type(PS2DeviceType &type, PS2Device dev) {
  const char *devname = dev == PS2Device::First ? "first" : "second";
  ps2::write(dev, dev_cmd::disable_scanning);
  auto response = read_input_response();
  if (response != dev_cmd::ack) {
    printf("[8042 PS/2] Could not disable scanning on %s device\n", devname);
    return false;
  }

  ps2::write(dev, dev_cmd::identify);
  response = read_input_response();
  if (response != dev_cmd::ack) {
    printf("[8042 PS/2] Identify command failed on %s device\n", devname);
    return false;
  }

  // FIXME: apparently the device can sometimes send two bytes
  const auto first_device_byte = read_input_response();
  switch (first_device_byte) {
  case 0x00:
    type = PS2DeviceType::MousePlain;
    return true;
  case 0x03:
    type = PS2DeviceType::MouseScrollWheel;
    return true;
  case 0x04:
    type = PS2DeviceType::MouseFiveButton;
    return true;
  case 0xAB: {
    const auto second_byte = read_input_response();
    switch (second_byte) {
    case 0x41:
    case 0xC1:
      type = PS2DeviceType::KeyboardWithTranslation;
      return true;
    case 0x83:
      type = PS2DeviceType::Keyboard;
      return true;
    default:
      printf("[8042 PS/2] Invalid device ident bytes 0x%.2X 0x%.2X\n",
             first_device_byte, second_byte);
      return false;
    }
  }
  default:
    printf("[8042 PS/2] Invalid device ident byte 0x%.2X 0x%.2X\n",
           first_device_byte);
    return false;
  }
}

void enable_interrupt(PS2Device dev) {
  auto response = read_command(cmd::read_configuration);
  response |= (dev == PS2Device::First ? 0x01 : 0x02);
  write_command(cmd::write_configuration, response);
}

} // namespace

namespace kernel::x86 {
static PS2Keyboard *s_keyboard;

// FIXME: Apparently it's a good idea to disable usb legacy support before doing
// this because otherwise we initialize the USB controller's PS/2 legacy mode
// instead. Probably have to look into this to get it to work with real
// hardware..
void init_ps2_controller() {
  // Refer to osdev wiki for proper steps. Some are skipped because lazy

  // Step 3. Disable both devices
  out_u8(command_register, cmd::disable_channel0);
  out_u8(command_register, cmd::disable_channel1);

  // Step 4. Flush output buffer
  in_u8(data_port); // discard

  // Step 5. Set controller configuration byte
  auto response = read_command(cmd::read_configuration);
  const bool is_double_channel = response & 0x20;
  if (is_double_channel) {
    puts("[8042 PS/2] Double channel controller");
  } else {
    puts("[8042 PS/2] Single channel controller");
  }
  response &= ~(0x43); // Disable IRQs and translation
  write_command(cmd::write_configuration, response);

  // Step 6. Perform controller self test
  out_u8(command_register, cmd::self_test);
  response = read_response();
  if (response == 0xFC) {
    KERNEL_PANIC("[8042 PS/2] Controller self test failed!");
  } else if (response == 0x55) {
    puts("[8042 PS/2] Controller self test succeeded!");
  } else {
    printf("[8042 PS/2] Unknown self-test response: 0x%.2X\n", response);
    UNREACHABLE();
  }

  // Step 8: Interface tests
  out_u8(command_register, cmd::test_channel0);
  bool first_device_available = false;
  response = read_response();
  if (response == 0x00) {
    puts("[8042 PS/2] Found first device");
    first_device_available = true;
  } else {
    printf("[8042 PS/2] Could not find first device, response 0x%.2X\n",
           response);
  }

  bool second_device_available = false;
  if (is_double_channel) {
    out_u8(command_register, cmd::test_channel1);
    response = read_response();
    if (response == 0x00) {
      puts("[8042 PS/2] Found second device");
      second_device_available = true;
    } else {
      printf("[8042 PS/2] Could not find second device, response 0x%.2X\n",
             response);
    }
  }

  auto dev_to_string = [](PS2DeviceType dev) -> const char * {
    switch (dev) {
    case PS2DeviceType::MousePlain:
      return "standard mouse";
    case PS2DeviceType::MouseFiveButton:
      return "5-button mouse";
    case PS2DeviceType::MouseScrollWheel:
      return "mouse with scrollwheel";
    case PS2DeviceType::Keyboard:
      return "keyboard";
    case PS2DeviceType::KeyboardWithTranslation:
      return "keyboard with translation";
    default:
      UNREACHABLE();
      return nullptr;
    }
  };

  bool found_keyboard = false;
  PS2Device keyboard_device = PS2Device::First;
  PS2DeviceType keyboard_type = PS2DeviceType::Keyboard;

  auto check_device = [&](PS2Device dev) {
    const auto enable_command =
        dev == PS2Device::First ? cmd::enable_channel0 : cmd::enable_channel1;
    out_u8(command_register, enable_command);
    ps2::write(dev, dev_cmd::reset);
    PS2DeviceType type;
    if (detect_device_type(type, dev)) {
      if (dev == PS2Device::First) {
        printf("[8042 PS/2] First device is a %s\n", dev_to_string(type));
      } else {
        printf("[8042 PS/2] Second device is a %s\n", dev_to_string(type));
      }
      if (type == PS2DeviceType::Keyboard ||
          type == PS2DeviceType::KeyboardWithTranslation) {
        found_keyboard = true;
        keyboard_type = type;
        keyboard_device = dev;
      }
    }
  };

  // Step 9: Enable devices
  if (first_device_available) {
    check_device(PS2Device::First);
  }
  if (second_device_available) {
    check_device(PS2Device::Second);
  }

  if (found_keyboard) {
    enable_interrupt(keyboard_device);
    s_keyboard = new PS2Keyboard(keyboard_device, keyboard_type);
  }
}

void ps2::write(const PS2Device &dev, u8 byte) {
  if (dev == PS2Device::Second) {
    out_u8(command_register, cmd::write_to_2nd_device);
  }

  // FIXME: We should probably have a timeout here
  while ((in_u8(status_register) & status::input_buffer_full) != 0) {
  }
  out_u8(data_port, byte);
}

u8 ps2::read_isr_response() { return in_u8(data_port); }

} // namespace kernel::x86
