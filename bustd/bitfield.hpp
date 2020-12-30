#pragma once

#include <bustd/assert.hpp>
#include <bustd/stddef.hpp>

namespace bu {
template <usize SIZE> class Bitfield {
  static_assert(SIZE != 0, "0-bit bitfields do not make sense");

public:
  Bitfield() {}
  // TODO: Convert from integer type

  bool operator[](usize i) const { return at(i); }
  bool at(usize i) const {
    ASSERT(i <= SIZE);
    const auto data_offset = i / 8;

    const auto data = m_data[data_offset];
    const auto byte_offset = i % 8;
    const auto shifted = 1 << byte_offset;
    return (data & shifted) != 0;
  }
  bool replace(usize i, bool v) {
    ASSERT(i <= SIZE);
    const auto data_offset = i / 8;

    auto &data = m_data[data_offset];
    const auto byte_offset = i % 8;
    const auto shifted = 1 << byte_offset;
    const auto old_value = (data & shifted) != 0;
    data &= ~shifted;
    if (v) {
      data |= shifted;
    }
    return old_value;
  }
  void set(usize i, bool v) { replace(i, v); }
  constexpr usize size() { return SIZE; }

private:
  u8 m_data[SIZE / 8 + (SIZE % 8 != 0 ? 1 : 0)]{};
};
} // namespace bu
