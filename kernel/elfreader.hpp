#pragma once

#include <bustd/stringview.hpp>
#include <kernel/process.hpp>

namespace kernel::elf {
using Entry = auto (*)() -> void;
auto parse(Process &proc, bu::StringView file) -> Entry;
} // namespace kernel::elf
