#pragma once

#include <bustd/assert.hpp>
#include <bustd/new.hpp>
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

  constexpr Optional(Optional &&other) { *this = forward(other); }
  constexpr Optional &operator=(Optional &&other) {
    if (m_has_data) {
      get_data()->~T();
    }

    m_has_data = other.m_has_data;
    other.m_has_data = false;
    if (m_has_data) {
      memcpy(m_data, other.m_data, sizeof(T));
    }
    return *this;
  }
  ~Optional() {
    if (m_has_data) {
      get_data()->~T();
    }
  }

  [[nodiscard]] constexpr explicit operator bool() const { return m_has_data; }

  [[nodiscard]] constexpr T &operator*() { return *get_data(); }
  [[nodiscard]] constexpr const T &operator*() const { return *get_data(); }

  constexpr const T *operator->() const { return get_data(); }
  constexpr T *operator->() { return get_data(); }

private:
  constexpr explicit Optional(nullptr_t) { m_has_data = false; }

  constexpr const T *get_data() const {
    ASSERT(m_has_data);
    return reinterpret_cast<const T *>(m_data);
  }
  constexpr T *get_data() {
    ASSERT(m_has_data);
    return reinterpret_cast<T *>(m_data);
  }

  alignas(T) u8 m_data[sizeof(T)]{};
  bool m_has_data{false};
};

template <typename T, class... Args>
[[nodiscard]] constexpr Optional<T> create_some(Args &&...args) {
  Optional<T> out = Optional<T>(nullptr);
  new (reinterpret_cast<T *>(&out.m_data)) T(forward(args)...);
  out.m_has_data = true;
  return forward(out);
}

template <typename T> [[nodiscard]] constexpr Optional<T> create_none() {
  return Optional<T>(nullptr);
}
} // namespace bu
