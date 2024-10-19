#pragma once

#include <bustd/assert.hpp>
#include <bustd/math.hpp>
#include <bustd/stddef.hpp>

#include <string.h>

namespace bu {
class StringView {
public:
  constexpr StringView() : StringView(nullptr){};
  constexpr StringView(nullptr_t) : m_data(nullptr), m_length(0){};
  constexpr StringView(const StringView &) = default;
  StringView(const char *s) : m_data(s), m_length(strlen(s)){};
  constexpr StringView(const char *s, const usize length)
      : m_data(s), m_length(length){};
  constexpr auto operator=(const StringView &) -> StringView & = default;
  constexpr auto operator=(StringView &&) -> StringView & = default;

  // Return s if this is null or empty.
  constexpr auto data_or(const char *s) const -> const char * {
    if (!*this) {
      return s;
    } else {
      return m_data;
    }
  }
  [[nodiscard]] constexpr auto len() const -> usize { return m_length; };
  [[nodiscard]] constexpr auto empty() const -> bool { return m_length == 0; }
  [[nodiscard]] constexpr auto data() const -> const char * { return m_data; };
  [[nodiscard]] inline auto data_u8() const -> const u8 * {
    return reinterpret_cast<const u8 *>(m_data);
  };

  [[nodiscard]] inline auto substr(const usize from,
                                   const usize upto) const -> StringView {
    ASSERT(from <= upto);
    if (from >= m_length) {
      return StringView("", 0);
    }
    return StringView(m_data + bu::min(from, m_length),
                      bu::min(upto - from, m_length - from));
  }

  constexpr auto operator==(const StringView &other) const -> bool {
    if (m_length != other.len()) {
      return false;
    }
    return strncmp(m_data, other.data(), m_length) == 0;
  }

  constexpr auto operator!=(const StringView &other) const -> bool {
    return !(*this == other);
  }

  inline auto operator[](const usize i) const -> char {
    ASSERT(i <= m_length);
    return m_data[i];
  }

  constexpr operator bool() const { return m_data != nullptr; }

private:
  const char *m_data;
  usize m_length;
};

namespace literals {
// We are the stdlib in this case, so we don't have to be nice
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wliteral-suffix"
constexpr auto operator"" sv(const char *data, usize length) -> StringView {
  return StringView(data, length);
}
#pragma GCC diagnostic pop
}; // namespace literals
} // namespace bu
