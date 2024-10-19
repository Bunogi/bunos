#include <kernel/filesystem/inodeindex.hpp>

namespace kernel::filesystem {
InodeIndex::InodeIndex(const u64 index) : m_index(index) {}
auto InodeIndex::get() const -> u64 { return m_index; }
} // namespace kernel::filesystem
