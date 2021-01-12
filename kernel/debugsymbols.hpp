#include <bustd/stddef.hpp>
#include <bustd/stringview.hpp>

namespace kernel {
void load_debug_symbols();
bu::StringView function_name_from_pc(u32 pc);
bool debug_symbols_loaded();
} // namespace kernel
