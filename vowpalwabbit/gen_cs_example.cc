/*
  Copyright (c) by respective owners including Yahoo!, Microsoft, and
  individual contributors. All rights reserved.  Released under a BSD (revised)
  license as described in the file LICENSE.
*/
#include <float.h>

#include "vw.h"
#include "reductions.h"
#include "cb_algs.h"
#include "vw_exception.h"
#include "gen_cs_example.h"

using namespace LEARNER;

using namespace CB;

inline bool observed_cost(cb_class* cl)
{ //cost observed for this action if it has non zero probability and cost != FLT_MAX
  return (cl != nullptr && cl->cost != FLT_MAX && cl->probability > .0);
}

cb_class* get_observed_cost(CB::label& ld)
{
  for (auto& cl : ld.costs)
    if (observed_cost(&cl))
      return &cl;
  return nullptr;
}

float get_unbiased_cost(CB::cb_class* observation, COST_SENSITIVE::label& scores, uint32_t action)
{
  for (auto& cl : scores.costs)
    if (cl.class_index == action)
      return get_unbiased_cost(observation, action, cl.x) + cl.x;
  return get_unbiased_cost(observation, action);
}

void gen_cs_example_ips(cb_to_cs& c, CB::label& ld, COST_SENSITIVE::label& cs_ld)
{ //this implements the inverse propensity score method, where cost are importance weighted by the probability of the chosen action
  //generate cost-sensitive example
  cs_ld.costs.erase();
  if (ld.costs.size() == 1 && !is_test_label(ld))   //this is a typical example where we can perform all actions
    { //in this case generate cost-sensitive example with all actions
      for (uint32_t i = 1; i <= c.num_actions; i++)
	{
	  COST_SENSITIVE::wclass wc;
	  wc.wap_value = 0.;
	  wc.x = 0.;
	  wc.class_index = i;
	  wc.partial_prediction = 0.;
	  wc.wap_value = 0.;
	  if (c.known_cost != nullptr && i == c.known_cost->action)
	    {
	      wc.x = c.known_cost->cost / c.known_cost->probability; //use importance weighted cost for observed action, 0 otherwise
	      //ips can be thought as the doubly robust method with a fixed regressor that predicts 0 costs for everything
	      //update the loss of this regressor
	      c.nb_ex_regressors++;
	      c.avg_loss_regressors += (1.0f / c.nb_ex_regressors)*((c.known_cost->cost)*(c.known_cost->cost) - c.avg_loss_regressors);
	      c.last_pred_reg = 0;
	      c.last_correct_cost = c.known_cost->cost;
	    }

	  cs_ld.costs.push_back(wc);
	}
    }
  else   //this is an example where we can only perform a subset of the actions
    { //in this case generate cost-sensitive example with only allowed actions
      for (auto& cl : ld.costs)
	{
	  COST_SENSITIVE::wclass wc;
	  wc.wap_value = 0.;
	  wc.x = 0.;
	  wc.class_index = cl.action;
	  wc.partial_prediction = 0.;
	  wc.wap_value = 0.;
	  if (c.known_cost != nullptr && cl.action == c.known_cost->action)
	    {
	      wc.x = c.known_cost->cost / c.known_cost->probability; //use importance weighted cost for observed action, 0 otherwise

	      //ips can be thought as the doubly robust method with a fixed regressor that predicts 0 costs for everything
	      //update the loss of this regressor
	      c.nb_ex_regressors++;
	      c.avg_loss_regressors += (1.0f / c.nb_ex_regressors)*((c.known_cost->cost)*(c.known_cost->cost) - c.avg_loss_regressors);
	      c.last_pred_reg = 0;
	      c.last_correct_cost = c.known_cost->cost;
	    }

	  cs_ld.costs.push_back(wc);
	}
    }

}


