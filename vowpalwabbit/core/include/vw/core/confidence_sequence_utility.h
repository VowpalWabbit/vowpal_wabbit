// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/action_score.h"
#include "vw/core/cb.h"
#include "vw/core/learner_fwd.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <vector>

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

/// Compute cost ranges [min_cost, max_cost] for each action using sensitivity analysis.
/// Used by both RegCB and SquareCB reductions.
///
/// \param delta Threshold on empirical loss difference (controls confidence width).
/// \param base The base learner used for sensitivity queries.
/// \param examples The multiline example set (action predictions are backed up and restored).
/// \param min_only If true, only compute minimum costs (used by RegCB optimistic variant).
/// \param min_cb_cost Lower bound on CB costs (e.g., 0).
/// \param max_cb_cost Upper bound on CB costs (e.g., 1).
/// \param min_costs Output vector of minimum cost for each action.
/// \param max_costs Output vector of maximum cost for each action (unchanged if min_only).
/// \param ex_as Scratch buffer for backing up action scores.
/// \param ex_costs Scratch buffer for backing up CB costs.
void get_cost_ranges(float delta, VW::LEARNER::learner& base, VW::multi_ex& examples, bool min_only,
    float min_cb_cost, float max_cb_cost, std::vector<float>& min_costs, std::vector<float>& max_costs,
    std::vector<VW::action_scores>& ex_as, std::vector<std::vector<VW::cb_class>>& ex_costs);

}  // namespace confidence_sequence_utility
}  // namespace VW
