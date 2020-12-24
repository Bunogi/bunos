#pragma once

namespace bu {
template <typename T> inline auto min(const T &lhs, const T &rhs) {
  if (lhs < rhs) {
    return lhs;
  } else {
    return rhs;
  }
}

template <typename T> inline auto max(const T &lhs, const T &rhs) {
  if (lhs > rhs) {
    return lhs;
  } else {
    return rhs;
  }
}
} // namespace bu
