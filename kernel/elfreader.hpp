#pragma once

#include <bustd/stringview.hpp>
#include <kernel/process.hpp>

namespace kernel::elf {
void (*parse(Process &proc, bu::StringView file))();
}
