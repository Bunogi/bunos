#pragma once

namespace bu {
template <typename T> inline constexpr auto min(const T &lhs, const T &rhs) {
  if (lhs < rhs) {
    return lhs;
  } else {
    return rhs;
  }
}

template <typename T> inline constexpr auto max(const T &lhs, const T &rhs) {
  if (lhs > rhs) {
    return lhs;
  } else {
    return rhs;
  }
}
} // namespace bu
