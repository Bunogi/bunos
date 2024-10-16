#pragma once

#include <bustd/function.hpp>
#include <bustd/optional.hpp>
#include <bustd/type_traits.hpp>

namespace bu {
// FIXME: use concepts to check if iterator :)
template <typename It>
It find_if(const It &first, const It &last,
           bu::Function<bool(const typename It::Value &)> f) {
  for (auto it = first; it != last; it++) {
    if (f(*it)) {
      return it;
    }
  }
  return last;
}
} // namespace bu
