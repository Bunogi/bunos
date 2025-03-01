#include <bustd/stddef.hpp>

namespace kernel {
void load_debug_symbols();
auto function_name_from_pc(usize pc) -> const char *;
} // namespace kernel
