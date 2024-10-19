#pragma once

#include <bustd/ownedptr.hpp>
#include <bustd/stddef.hpp>
#include <bustd/type_traits.hpp>

namespace bu {
template <class> class Function;
template <class T, class... Args> class Function<T(Args...)> {
private:
  struct CallableBase {
    virtual auto call(Args...) -> T = 0;
    virtual ~CallableBase() {}
  };

  template <typename F> struct Callable final : CallableBase {
  public:
    F m_functor;
    Callable(F functor) : m_functor(functor) {}
    virtual auto call(Args... a) -> T override {
      return m_functor(bu::forward(a)...);
    }
  };

  bu::OwnedPtr<CallableBase> m_func;

public:
  Function();
  // TODO: Should check if is function as well
  template <class F, class = typename enable_if<is_pointer<F>>::type>
  Function(F f) : m_func(new Callable<F>(move(f))) {}
  template <class F, class = typename enable_if<!is_pointer<F>>::type>
  Function(F &&f) : m_func(new Callable<F>(move(f))) {}
  auto operator()(Args... a) -> T { return m_func->call(a...); }
};
} // namespace bu
