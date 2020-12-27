#include "string_view.hpp"

#include <bustd/math.hpp>
#include <string.h>

namespace bu {
StringView::StringView() : StringView(nullptr) {}
StringView::StringView(nullptr_t) : m_data(nullptr), m_length(0) {}
StringView::StringView(const char *s) : m_data(s), m_length(strlen(s)) {}
StringView::StringView(const char *s, const usize length)
    : m_data(length == 0 ? nullptr : s), m_length(length) {}

const char *StringView::data_or(const char *s) const {
  if (m_data == nullptr || m_length == 0) {

    return s;
  } else {
    return m_data;
  }
}
const char *StringView::data() const { return m_data; }
const u8 *StringView::data_u8() const {
  return reinterpret_cast<const u8 *>(m_data);
}
usize StringView::len() const { return m_length; }

bool StringView::operator==(const StringView &other) const {
  if (m_length != other.len()) {
    return false;
  }
  return strcmp(m_data, other.data()) == 0;
}

bool StringView::operator!=(const StringView &other) const {
  return !(*this == other);
}

StringView StringView::first_part(const usize upto) const {
  return StringView(m_data, upto);
}

StringView StringView::last_part(const usize from) const {
  return StringView(m_data + from, m_length - from);
}

StringView StringView::substr(const usize from, const usize upto) const {
  return StringView(m_data + from, bu::min(upto - from, m_length));
}

} // namespace bu
