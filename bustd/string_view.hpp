#pragma once

#include <bustd/stddef.hpp>

namespace bu {
class StringView {
public:
  StringView();
  StringView(nullptr_t);
  StringView(const char *s);
  StringView(const char *s, const usize length);

  // Return s if this is null or empty.
  const char *data_or(const char *s) const;
  usize len() const;
  const char *data() const;
  const u8 *data_u8() const;

  StringView first_part(const usize upto) const;
  StringView substr(const usize from, const usize upto) const;
  StringView last_part(const usize from) const;

  bool operator==(const StringView &other) const;
  bool operator!=(const StringView &other) const;

private:
  const char *m_data;
  usize m_length;
};
} // namespace bu
