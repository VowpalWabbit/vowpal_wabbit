/*
  Copyright (c) by respective owners including Yahoo!, Microsoft, and
  individual contributors. All rights reserved.  Released under a BSD (revised)
  license as described in the file LICENSE.
*/
#include <float.h>

#include "vw.h"
#include "reductions.h"
#include "vw_exception.h"
#include "gen_cs_example.h"

namespace GEN_CS
{
using namespace LEARNER;
using namespace CB_ALGS;
using namespace CB;

inline bool observed_cost(cb_class* cl)
{ //cost observed for this action if it has non zero probability and cost != FLT_MAX
  return (cl != nullptr && cl->cost != FLT_MAX && cl->probability > .0);
}

cb_class* get_observed_cost(CB::label& ld)
{ for (auto& cl : ld.costs)
    if (observed_cost(&cl))
      return &cl;
  return nullptr;
}

  float safe_probability(float prob)
  {
    if (prob <= 0.)
      {
	std::cout << "Probability " << prob << " is not possible, replacing with 1e-3.  Fix your dataset. " << std::endl;
	return 1e-3;
      }
    else
      return prob;
  }
  
//Multiline version
void gen_cs_example_ips(v_array<example*> examples, COST_SENSITIVE::label& cs_labels)
{ cs_labels.costs.erase();
  bool shared = CB::ec_is_example_header(*examples[0]);
  for (uint32_t i = 0; i < examples.size()-1; i++)
  { CB::label ld = examples[i]->l.cb;

    COST_SENSITIVE::wclass wc = {0.,i,0.,0.};
    if (shared && i > 0)
      wc.class_index = (uint32_t)i-1;
    if (ld.costs.size() == 1 && ld.costs[0].cost != FLT_MAX)
      wc.x = ld.costs[0].cost / safe_probability(ld.costs[0].probability);
    cs_labels.costs.push_back(wc);
  }

  if (shared)//take care of shared examples
  { cs_labels.costs[0].class_index = 0;
    cs_labels.costs[0].x = -FLT_MAX;
  }
}

//Multiline version
void gen_cs_example_dm(v_array<example*> examples, COST_SENSITIVE::label& cs_labels)
{ cs_labels.costs.erase();
  bool shared = CB::ec_is_example_header(*examples[0]);
  for (uint32_t i = 0; i < examples.size()-1; i++)
  { CB::label ld = examples[i]->l.cb;

    COST_SENSITIVE::wclass wc = {0.,i,0.,0.};
    if (shared && i > 0)
      wc.class_index = (uint32_t)i-1;
    if (ld.costs.size() == 1 && ld.costs[0].cost != FLT_MAX)
      wc.x = ld.costs[0].cost;
    cs_labels.costs.push_back(wc);
  }

  if (shared)//take care of shared examples
  { cs_labels.costs[0].class_index = 0;
    cs_labels.costs[0].x = -FLT_MAX;
  }
}

//Multiline version
void gen_cs_test_example(v_array<example*> examples, COST_SENSITIVE::label& cs_labels)
{ cs_labels.costs.erase();
  bool shared = CB::ec_is_example_header(*examples[0]);
  for (uint32_t i = 0; i < examples.size()-1; i++)
  { COST_SENSITIVE::wclass wc = {FLT_MAX,i,0.,0.};
    if (shared && i > 0)
      wc.class_index = (uint32_t)i-1;
    cs_labels.costs.push_back(wc);
  }

  if (shared)//take care of shared examples
  { cs_labels.costs[0].class_index = 0;
    cs_labels.costs[0].x = -FLT_MAX;
  }
}

//single line version
void gen_cs_example_ips(cb_to_cs& c, CB::label& ld, COST_SENSITIVE::label& cs_ld)
{ //this implements the inverse propensity score method, where cost are importance weighted by the probability of the chosen action
  //generate cost-sensitive example
  cs_ld.costs.erase();
  if (ld.costs.size() == 1 && !is_test_label(ld))   //this is a typical example where we can perform all actions
  { //in this case generate cost-sensitive example with all actions
    for (uint32_t i = 1; i <= c.num_actions; i++)
    { COST_SENSITIVE::wclass wc = {0.,i,0.,0.};
      if (c.known_cost != nullptr && i == c.known_cost->action)
      {
	wc.x = c.known_cost->cost / safe_probability(c.known_cost->probability); //use importance weighted cost for observed action, 0 otherwise
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
    { COST_SENSITIVE::wclass wc = {0., cl.action, 0., 0.};
      if (c.known_cost != nullptr && cl.action == c.known_cost->action)
	{ wc.x = c.known_cost->cost / safe_probability(c.known_cost->probability); //use importance weighted cost for observed action, 0 otherwise

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

void gen_cs_example_mtr(cb_to_cs_adf& c, v_array<example*>& ec_seq, COST_SENSITIVE::label& cs_labels)
{ bool shared = CB::ec_is_example_header(*(ec_seq[0]));
  c.action_sum += ec_seq.size()-2; //-1 for shared -1 for end example
  if (!shared)
    c.action_sum += 1;
  c.event_sum++;

  c.mtr_ec_seq.erase();
  cs_labels.costs.erase();
  for (size_t i = 0; i < ec_seq.size(); i++)
  { CB::label ld = ec_seq[i]->l.cb;

    COST_SENSITIVE::wclass wc = {0, 0};

    bool keep_example = false;
    if (shared && i == 0)
    { wc.x = -FLT_MAX;
      keep_example = true;
    }
    else if (ld.costs.size() == 1 && ld.costs[0].cost != FLT_MAX)
    { wc.x = ld.costs[0].cost;
      c.mtr_example = (uint32_t)i;
      keep_example = true;
    }
    if (keep_example)
    { c.mtr_ec_seq.push_back(ec_seq[i]);
      cs_labels.costs.push_back(wc);
    }
  }
  c.mtr_ec_seq.push_back(ec_seq[ec_seq.size()-1]);//must include the end-of-line example
}
}
