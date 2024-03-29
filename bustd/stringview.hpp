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
  constexpr StringView &operator=(const StringView &) = default;
  constexpr StringView &operator=(StringView &&) = default;

  // Return s if this is null or empty.
  constexpr const char *data_or(const char *s) const {
    if (!*this) {
      return s;
    } else {
      return m_data;
    }
  }
  constexpr usize len() const { return m_length; };
  constexpr bool empty() const { return m_length == 0; }
  constexpr const char *data() const { return m_data; };
  inline const u8 *data_u8() const {
    return reinterpret_cast<const u8 *>(m_data);
  };

  inline StringView substr(const usize from, const usize upto) const {
    ASSERT(from <= upto);
    if (from >= m_length) {
      return StringView("", 0);
    }
    return StringView(m_data + bu::min(from, m_length),
                      bu::min(upto - from, m_length - from));
  }

  constexpr bool operator==(const StringView &other) const {
    if (m_length != other.len()) {
      return false;
    }
    return strncmp(m_data, other.data(), m_length) == 0;
  }

  constexpr bool operator!=(const StringView &other) const {
    return !(*this == other);
  }

  inline char operator[](const usize i) const {
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
constexpr StringView operator"" sv(const char *data, usize length) {
  return StringView(data, length);
}
#pragma GCC diagnostic pop
}; // namespace literals
} // namespace bu
