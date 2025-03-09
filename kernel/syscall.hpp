#pragma once

#include <kernel/process.hpp>
#include <kernel/task/arch.hpp>

namespace kernel {
// FIXME: Parts of this is generic and can be moved out
class SyscallArg {
public:
  SyscallArg(const usize v) : m_inner(v) {}

  auto raw() -> usize { return m_inner; }

private:
  usize m_inner;
};

class SyscallArguments {
public:
  SyscallArguments(const usize num, const usize arg1, const usize arg2,
                   const usize arg3)
      : m_args{num, arg1, arg2, arg3} {}

  [[nodiscard]] auto get(const usize num) const -> const SyscallArg & {
    ASSERT(num < m_max_args);
    return m_args[num];
  }

private:
  constexpr static auto m_max_args = 4;
  SyscallArg m_args[m_max_args];
};

void syscall_handler(const SyscallArguments &args, Process *const target_proc);

} // namespace kernel
