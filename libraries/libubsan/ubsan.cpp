
// To add stuff here, look in the GCC source code under libsanitizer. Look for
// the matching function call in ubsan/ubsan_handler.cpp and the header file
//

#include <bustd/stddef.hpp>
#include <kernel/panic.hpp>
#include <libc/stdio.h>

#define MAKE_DEF(foo)                                                          \
  void __ubsan_handle_##foo() { KERNEL_PANIC(#foo); }

extern "C" {

MAKE_DEF(add_overflow);
MAKE_DEF(divrem_overflow);
MAKE_DEF(load_invalid_value);
MAKE_DEF(mul_overflow);
MAKE_DEF(out_of_bounds);
MAKE_DEF(pointer_overflow);
MAKE_DEF(shift_out_of_bounds);
MAKE_DEF(sub_overflow);
}
