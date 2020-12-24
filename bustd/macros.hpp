#pragma once

#define BU_NOCOPY(__x)                                                         \
  __x(const __x &) = delete;                                                   \
  __x operator=(const __x &) = delete;
