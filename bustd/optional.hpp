#pragma once

#include <bustd/assert.hpp>
#include <bustd/stddef.hpp>
#include <bustd/type_traits.hpp>
#include <libc/string.h>

namespace bu {
template <typename T> class Optional {
  static_assert(!is_same<T, bu::nullptr_t>::value);

public:
  template <typename U, class... Args>
  friend constexpr Optional<U> create_some(Args &&...);
  template <typename U> friend constexpr Optional<U> create_none();

  constexpr Optional(Optional &&other) {
    m_has_data = other.m_has_data;
    other.m_has_data = false;
    if (m_has_data) {
      memcpy(m_data, other.m_data, sizeof(T));
    }
  }
  constexpr Optional &operator=(Optional &&other) { *this = other; }
  ~Optional() {
    if (m_has_data) {
      reinterpret_cast<T *>(m_data)->~T();
    }
  }

  [[nodiscard]] constexpr explicit operator bool() const { return m_has_data; }

  [[nodiscard]] constexpr T &operator*() {
    ASSERT(m_has_data);
    return *reinterpret_cast<T *>(m_data);
  }
  [[nodiscard]] constexpr const T &operator*() const {
    ASSERT(m_has_data);
    return *reinterpret_cast<const T *>(m_data);
  }

private:
  constexpr explicit Optional(T &&data) {
    memcpy(m_data, reinterpret_cast<u8 *>(&data), sizeof(T));
    m_has_data = true;
  }
  constexpr explicit Optional(nullptr_t) { m_has_data = false; }

  alignas(T) u8 m_data[sizeof(T)];
  bool m_has_data{false};
};

template <typename T, class... Args>
[[nodiscard]] constexpr Optional<T> create_some(Args &&...args) {
  return Optional<T>(T(forward(args)...));
}

template <typename T> [[nodiscard]] constexpr Optional<T> create_none() {
  return Optional<T>(nullptr);
}
} // namespace bu
