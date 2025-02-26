
// To add stuff here, look in the GCC source code under libsanitizer. Look for
// the matching function call in ubsan/ubsan_handler.cpp and the header file
//

#include <bustd/stddef.hpp>
#include <kernel/panic.hpp>
#include <libc/stdio.h>

// FIXME: This breaks on non-x86.. I think
using uptr = unsigned long;
using ValueHandle = uptr;

#define MAKE_DEF(foo)                                                          \
  void __ubsan_handle_##foo() { KERNEL_PANIC(#foo); }

struct SourceLocation {
  const char *filename;
  u32 line;
  u32 column;
};

struct TypeDescriptor {
  u16 kind;
  u16 info;
  char type_name[];
};

struct TypeMismatchData {
  SourceLocation loc;
  const TypeDescriptor &type;
  unsigned char log_align;
  unsigned char typecheck_kind;
};

void dump_location(const SourceLocation &location) {
  printf("%s:%d:%d", location.filename, location.line, location.column);
}

extern "C" {

//
void __ubsan_handle_type_mismatch_v1(TypeMismatchData *const data,
                                     usize pointer) {
  printf("Type mismatch: ");
  dump_location(data->loc);
  printf("\n");
  KERNEL_PANIC("UBSan detected an error");
}

MAKE_DEF(add_overflow);
MAKE_DEF(divrem_overflow);
MAKE_DEF(load_invalid_value);
MAKE_DEF(mul_overflow);
MAKE_DEF(out_of_bounds);
MAKE_DEF(pointer_overflow);
MAKE_DEF(shift_out_of_bounds);
MAKE_DEF(sub_overflow);
}
