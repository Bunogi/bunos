#include <bustd/stddef.hpp>

namespace kernel {
void load_debug_symbols();
auto function_name_from_pc(u32 pc) -> const char *;
auto debug_symbols_loaded() -> bool;
} // namespace kernel
