#include <bustd/stddef.hpp>
#include <bustd/stringview.hpp>

namespace kernel {
void load_debug_symbols();
auto function_name_from_pc(u32 pc) -> bu::StringView;
auto debug_symbols_loaded() -> bool;
} // namespace kernel
