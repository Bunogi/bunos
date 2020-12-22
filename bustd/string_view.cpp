#include "string_view.hpp"

#include <string.h>

namespace bu {
StringView::StringView(nullptr_t) : m_data(nullptr), m_length(0) {}
StringView::StringView(const char *s) : m_data(s), m_length(strlen(s)) {}
StringView::StringView(const char *s, usize length)
    : m_data(s), m_length(length) {}

const char *StringView::data_or(const char *s) const {
  if (m_data == nullptr || m_length == 0) {

    return s;
  } else {
    return m_data;
  }
}
const char *StringView::data() const { return m_data; }
usize StringView::length() const { return m_length; }
} // namespace bu
