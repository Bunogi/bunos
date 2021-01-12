#pragma once

#include <bustd/stddef.hpp>
#include <bustd/stringview.hpp>
#include <bustd/vector.hpp>

namespace kernel {
class IFileSystem {
public:
  virtual ~IFileSystem() = default;
  // FIXME: Find a good interface for this
  virtual isize read_file(bu::StringView file, u8 *buffer, u64 offset,
                          usize len) = 0;
  virtual bu::Vector<bu::Vector<char>> read_dir(bu::StringView path) = 0;
  virtual u64 file_size(bu::StringView file) = 0;
};
} // namespace kernel
