#pragma once

#include <bustd/stddef.hpp>

namespace bu {
class StringView {
public:
  StringView(nullptr_t);
  StringView(const char *s);
  StringView(const char *s, usize length);

  // Return s if this is null or empty.
  const char *data_or(const char *s) const;
  usize length() const;
  const char *data() const;

private:
  const char *m_data;
  usize m_length;
};
} // namespace bu
