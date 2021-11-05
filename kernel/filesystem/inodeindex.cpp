#include <kernel/filesystem/inodeindex.hpp>

namespace kernel::filesystem {
InodeIndex::InodeIndex(const u64 index) : m_index(index) {}
u64 InodeIndex::get() const { return m_index; }
} // namespace kernel::filesystem
