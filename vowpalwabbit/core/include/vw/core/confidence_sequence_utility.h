// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include <algorithm>
#include <cfloat>
#include <cmath>

namespace VW
{
namespace confidence_sequence_utility
{
constexpr int BINARY_SEARCH_MAX_ITER = 20;

/// Binary search to find the largest weight w such that
/// w * (fhat^2 - (fhat - w * sens)^2) <= delta.
/// See Section 7.1 in https://arxiv.org/pdf/1703.01014.pdf.
inline float binary_search(float fhat, float delta, float sens, float tol = 1e-6f)
{
  const float maxw = (std::min)(fhat / sens, FLT_MAX);
  if (maxw * fhat * fhat <= delta) { return maxw; }

  float l = 0;
  float u = maxw;
  float w, v;

  for (int iter = 0; iter < BINARY_SEARCH_MAX_ITER; iter++)
  {
    w = (u + l) / 2.f;
    v = w * (fhat * fhat - (fhat - sens * w) * (fhat - sens * w)) - delta;
    if (v > 0) { u = w; }
    else { l = w; }
    if (std::fabs(v) <= tol || u - l <= tol) { break; }
  }

  return l;
}
}  // namespace confidence_sequence_utility
}  // namespace VW
