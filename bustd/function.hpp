#pragma once

#include <bustd/ownedptr.hpp>
#include <bustd/stddef.hpp>
#include <bustd/type_traits.hpp>

namespace bu {
template <class> class Function;
template <class T, class... Args> class Function<T(Args...)> {
private:
  struct CallableBase {
    virtual T call(Args...) = 0;
    virtual ~CallableBase() {}
  };

  template <typename F> struct Callable final : CallableBase {
  public:
    F m_functor;
    Callable(F functor) : m_functor(functor) {}
    virtual T call(Args... a) override { return m_functor(bu::forward(a)...); }
  };

  bu::OwnedPtr<CallableBase> m_func;

public:
  Function();
  template <class F> Function(F f) : m_func(new Callable<F>(move(f))) {}
  T operator()(Args... a) { return m_func->call(a...); }
};
} // namespace bu
