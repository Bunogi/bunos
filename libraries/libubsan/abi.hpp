#pragma once

#include <bustd/stddef.hpp>

namespace ubsan {

struct SourceLocation {
  const char *filename;
  u32 line;
  u32 column;
};

struct TypeDescriptor {
  u16 kind;
  u16 info;
  char *type_name;
};

} // namespace ubsan
