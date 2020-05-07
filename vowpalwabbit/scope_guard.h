#pragma once

#include <utility>
#include "future_compat.h"

namespace VW
{
namespace details
{
template <typename TScopeExitLambda>
class scope_guard_caller
{
 public:
  explicit scope_guard_caller(TScopeExitLambda&& lambda) noexcept : _scope_exit_lambda(std::move(lambda))
  {
    static_assert(std::is_same<decltype(lambda()), void>::value, "scope_guard lambdas cannot have a return value.");
  }

  scope_guard_caller(const scope_guard_caller&) = delete;
  scope_guard_caller& operator=(const scope_guard_caller&) = delete;

  scope_guard_caller(scope_guard_caller&& other) noexcept
      : _scope_exit_lambda(std::move(other._scope_exit_lambda)), _will_call(other._will_call)
  {
    other._will_call = false;
  }

  scope_guard_caller& operator=(scope_guard_caller&& other) noexcept
  {
    if (this == &other)
    {
      return *this;
    }

    _scope_exit_lambda = std::move(other._scope_exit_lambda);
    _will_call = other._will_call;
    other._will_call = false;
    return *this;
  }

  ~scope_guard_caller() noexcept { call(); }

  void cancel() noexcept { _will_call = false; }
  void call() noexcept
  {
    if (_will_call == true)
    {
      _will_call = false;
      _scope_exit_lambda();
    }
  }

 private:
  bool _will_call = true;
  TScopeExitLambda _scope_exit_lambda;
};

}  // namespace details

template <typename TScopeExitLambda>
VW_ATTR(nodiscard) inline details::scope_guard_caller<TScopeExitLambda> scope_guard(TScopeExitLambda&& lambda) noexcept
{
  return details::scope_guard_caller<TScopeExitLambda>(std::forward<TScopeExitLambda>(lambda));
}

}  // namespace VW
