// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/confidence_sequence_utility.h"

#include "vw/core/learner.h"

#include <cmath>

void VW::confidence_sequence_utility::get_cost_ranges(float delta, VW::LEARNER::learner& base,
    VW::multi_ex& examples, bool min_only, float min_cb_cost, float max_cb_cost, std::vector<float>& min_costs,
    std::vector<float>& max_costs, std::vector<VW::action_scores>& ex_as,
    std::vector<std::vector<VW::cb_class>>& ex_costs)
{
  const size_t num_actions = examples[0]->pred.a_s.size();
  min_costs.resize(num_actions);
  max_costs.resize(num_actions);

  ex_as.clear();
  ex_costs.clear();

  // backup cb example data
  for (const auto& ex : examples)
  {
    ex_as.push_back(ex->pred.a_s);
    ex_costs.push_back(ex->l.cb.costs);
  }

  // set regressor predictions
  for (const auto& as : ex_as[0]) { examples[as.action]->pred.scalar = as.score; }

  const float cmin = min_cb_cost;
  const float cmax = max_cb_cost;

  for (size_t a = 0; a < num_actions; ++a)
  {
    auto* ec = examples[a];
    ec->l.simple.label = cmin - 1;
    float sens = base.sensitivity(*ec);
    float w = 0;  // importance weight

    if (ec->pred.scalar < cmin || std::isnan(sens) || std::isinf(sens)) { min_costs[a] = cmin; }
    else
    {
      w = VW::confidence_sequence_utility::binary_search(ec->pred.scalar - cmin + 1, delta, sens);
      min_costs[a] = (std::max)(ec->pred.scalar - sens * w, cmin);
      if (min_costs[a] > cmax) { min_costs[a] = cmax; }
    }

    if (!min_only)
    {
      ec->l.simple.label = cmax + 1;
      sens = base.sensitivity(*ec);
      if (ec->pred.scalar > cmax || std::isnan(sens) || std::isinf(sens)) { max_costs[a] = cmax; }
      else
      {
        w = VW::confidence_sequence_utility::binary_search(cmax + 1 - ec->pred.scalar, delta, sens);
        max_costs[a] = (std::min)(ec->pred.scalar + sens * w, cmax);
        if (max_costs[a] < cmin) { max_costs[a] = cmin; }
      }
    }
  }

  // reset cb example data
  for (size_t i = 0; i < examples.size(); ++i)
  {
    examples[i]->pred.a_s = ex_as[i];
    examples[i]->l.cb.costs = ex_costs[i];
  }
}
