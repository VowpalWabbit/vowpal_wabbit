// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/common/future_compat.h"

#include <utility>

// While it is a general concept this specific implementation of scope based cleanup was inspired by
// https://github.com/microsoft/wil/blob/cd51fa9d3c24e9e9f5d4cdf1a9768d68f441fc48/include/wil/resource.h#L543

namespace VW
{
namespace details
{
template <typename TScopeExitLambda>
class scope_exit_caller
{
public:
  explicit scope_exit_caller(TScopeExitLambda&& lambda) noexcept : _scope_exit_lambda(std::move(lambda))
  {
    static_assert(std::is_same<decltype(lambda()), void>::value, "scope_exit lambdas cannot have a return value.");
  }

  scope_exit_caller(const scope_exit_caller&) = delete;
  scope_exit_caller& operator=(const scope_exit_caller&) = delete;
  scope_exit_caller& operator=(scope_exit_caller&& other) = delete;

  scope_exit_caller(scope_exit_caller&& other) noexcept
      : _will_call(other._will_call), _scope_exit_lambda(std::move(other._scope_exit_lambda))
  {
    other._will_call = false;
  }

  ~scope_exit_caller() noexcept { call(); }

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

/// Created an RAII object which executes the provided lambda when the scope exits.
/// The primary use case is to handle cleanup in code where exceptions are possible but the code is not exception safe.
///
/// #### Example:
/// \code
/// {
///   auto* resource = /* some_resource_that_needs_cleanup */;
///   auto guard = VW::scope_exit([resource]() { /* cleanup_resource(resource); */ });
/// }
/// // Lambda has executed at this point.
/// \endcode
template <typename TScopeExitLambda>
VW_ATTR(nodiscard)
inline details::scope_exit_caller<TScopeExitLambda> scope_exit(TScopeExitLambda&& lambda) noexcept
{
  return details::scope_exit_caller<TScopeExitLambda>(std::forward<TScopeExitLambda>(lambda));
}

}  // namespace VW
