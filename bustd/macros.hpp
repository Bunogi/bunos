#pragma once

// clang-format off
#define BU_NOCOPY(__x)                                                         \
  __x(const __x &) = delete;                                                   \
  auto operator=(const __x &) -> __x & = delete;

#define BU_NOMOVE(__x)                                                         \
  __x(__x &&) = delete;                                                        \
  auto operator=(__x &&) -> __x && = delete;
// clang-format on
