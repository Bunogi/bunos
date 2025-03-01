#pragma once

#include <libubsan/abi.hpp>

namespace ubsan {
void abort(const char *violation, const char *during,
           const SourceLocation &location, usize address);
}
