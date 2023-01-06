// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// On Windows, exp(-infinity) incorrectly returns -infinity instead of 0.
// So we replace it with our own version that checks for this condition.

#pragma once

#include <cmath>

namespace VW
{
namespace details
{

#ifdef _WIN32
// this is a bug in VS2013, fixed in VS2015 runtime
template <typename T>
T correctedExp(T exponent)
{
  if (isinf(exponent) && exponent < T(0)) { return T(0); }
  else { return std::exp(exponent); }
}
#else
// std::exp is used because on Linux, not using the namespace caused a different implementation of
// exp to be linked providing incorrect values when `#include <boost/program_options.hpp>` was
// removed in global_data.h
template <typename T>
T correctedExp(T exponent)
{
  return std::exp(exponent);
}
#endif

}  // namespace details
}  // namespace VW
