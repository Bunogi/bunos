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
  friend constexpr auto create_some(Args &&...) -> Optional<U>;
  template <typename U> friend constexpr auto create_none() -> Optional<U>;

  constexpr Optional(Optional &&other) { *this = forward(other); }
  constexpr auto operator=(Optional &&other) -> Optional & {
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

  [[nodiscard]] constexpr auto operator*() -> T & { return *get_data(); }
  [[nodiscard]] constexpr auto operator*() const -> const T & {
    return *get_data();
  }

  constexpr auto operator->() const -> const T * { return get_data(); }
  constexpr auto operator->() -> T * { return get_data(); }

private:
  constexpr explicit Optional(nullptr_t) { m_has_data = false; }

  [[nodiscard]] constexpr auto get_data() const -> const T * {
    ASSERT(m_has_data);
    return reinterpret_cast<const T *>(m_data);
  }
  constexpr auto get_data() -> T * {
    ASSERT(m_has_data);
    return reinterpret_cast<T *>(m_data);
  }

  alignas(T) u8 m_data[sizeof(T)]{};
  bool m_has_data{false};
};

template <typename T, class... Args>
[[nodiscard]] constexpr auto create_some(Args &&...args) -> Optional<T> {
  Optional<T> out = Optional<T>(nullptr);
  new (reinterpret_cast<T *>(&out.m_data)) T(forward(args)...);
  out.m_has_data = true;
  return forward(out);
}

template <typename T>
[[nodiscard]] constexpr auto create_none() -> Optional<T> {
  return Optional<T>(nullptr);
}
} // namespace bu
