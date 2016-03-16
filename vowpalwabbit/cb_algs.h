/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
//TODO: extend to handle CSOAA_LDF and WAP_LDF
LEARNER::base_learner* cb_algs_setup(vw& all);

#define CB_TYPE_DR 0
#define CB_TYPE_DM 1
#define CB_TYPE_IPS 2

namespace CB_ALGS
{
template <bool is_learn>
float get_cost_pred(LEARNER::base_learner* scorer, CB::cb_class* known_cost, example& ec, uint32_t index, uint32_t base)
{ CB::label ld = ec.l.cb;

  label_data simple_temp;
  simple_temp.initial = 0.;
  if (known_cost != nullptr && index == known_cost->action)
    simple_temp.label = known_cost->cost;
  else
    simple_temp.label = FLT_MAX;

  ec.l.simple = simple_temp;
  polyprediction p = ec.pred;

  if (is_learn && simple_temp.label != FLT_MAX)
  { float old_weight = ec.weight;
    if (known_cost != nullptr && index == known_cost->action)
      ec.weight = 1.f / known_cost->probability;
    else
      ec.weight = 1.f;
    scorer->learn(ec, index-1+base);
    ec.weight = old_weight;
  }
  else
    scorer->predict(ec, index-1+base);

  float pred = ec.pred.scalar;
  ec.pred = p;

  ec.l.cb = ld;

  return pred;
}


}

float get_unbiased_cost(CB::cb_class* known_cost, COST_SENSITIVE::label& cb_label, uint32_t action);
inline float get_unbiased_cost(CB::cb_class* observation, uint32_t action, float offset = 0.) 
{
  if (action == observation->action)
    return (observation->cost - offset) / observation->probability;
  return 0.;
}

