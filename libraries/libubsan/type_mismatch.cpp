#include <libc/stdio.h>
#include <libstacktrace/stacktrace.hpp>
#include <libubsan/abi.hpp>
#include <libubsan/abort.hpp>

namespace ubsan {
struct TypeMismatchData {
  SourceLocation loc;
  const TypeDescriptor &type;
  unsigned char log_align;
  unsigned char typecheck_kind;
};

enum class TypeCheckKind : u8 {
  Load,
  Store,
  RefBinding,
  MemberAccess,
  MemberCall,
  ConstructorCall,
  DownCastPointer,
  DownCastReference,
  Upcast,
  UpcastToVirtualBase,
  NonNullAssign,
  DynamicOperation
};

// Corresponds to the above
const char *const TypeCheckExplanations[] = {"load of",
                                             "store to",
                                             "reference binding to",
                                             "member access within",
                                             "member call on",
                                             "constructor call on",
                                             "downcast of",
                                             "downcast of",
                                             "upcast of",
                                             "cast to virtual base of",
                                             "_Nonnull binding to",
                                             "dynamic operation on"};
} // namespace ubsan

extern "C" {
void __ubsan_handle_type_mismatch_v1(ubsan::TypeMismatchData *const data,
                                     const usize pointer) {
  const char *violation = "type mismatch";
  const usize alignment = (usize)1 << data->log_align;

  if (!pointer) {
    violation = "null pointer access";
  } else if (alignment && (pointer & (alignment - 1))) {
    violation = "misaligned access";
  }

  ubsan::abort(violation, ubsan::TypeCheckExplanations[data->typecheck_kind],
               data->loc, pointer);
}
}
