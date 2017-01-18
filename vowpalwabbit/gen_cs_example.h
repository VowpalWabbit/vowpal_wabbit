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

namespace GEN_CS
{

struct cb_to_cs
{ size_t cb_type;
  uint32_t num_actions;
  COST_SENSITIVE::label pred_scores;
  LEARNER::base_learner* scorer;
  float avg_loss_regressors;
  size_t nb_ex_regressors;
  float last_pred_reg;
  float last_correct_cost;

  CB::cb_class* known_cost;
};

struct cb_to_cs_adf
{ size_t cb_type;

  //for MTR
  uint64_t action_sum;
  uint64_t event_sum;
  uint32_t mtr_example;
  v_array<example*> mtr_ec_seq;//shared + the one example + an end example.

  //for DR
  COST_SENSITIVE::label pred_scores;
  CB::cb_class known_cost;
  LEARNER::base_learner* scorer;
};

CB::cb_class* get_observed_cost(CB::label& ld);

void gen_cs_example_ips(cb_to_cs& c, CB::label& ld, COST_SENSITIVE::label& cs_ld);

template <bool is_learn>
void gen_cs_example_dm(cb_to_cs& c, example& ec, COST_SENSITIVE::label& cs_ld)
{ //this implements the direct estimation method, where costs are directly specified by the learned regressor.
  CB::label ld = ec.l.cb;

  float min = FLT_MAX;
  uint32_t argmin = 1;
  //generate cost sensitive example
  cs_ld.costs.erase();
  c.pred_scores.costs.erase();

  if (ld.costs.size() == 1 && !is_test_label(ld))   //this is a typical example where we can perform all actions
  { //in this case generate cost-sensitive example with all actions
    for (uint32_t i = 1; i <= c.num_actions; i++)
    { COST_SENSITIVE::wclass wc = {0., i, 0., 0.};
      //get cost prediction for this action
      wc.x = CB_ALGS::get_cost_pred<is_learn>(c.scorer, c.known_cost, ec, i, 0);
      if (wc.x < min)
      { min = wc.x;
        argmin = i;
      }

      c.pred_scores.costs.push_back(wc);

      if (c.known_cost != nullptr && c.known_cost->action == i)
      { c.nb_ex_regressors++;
        c.avg_loss_regressors += (1.0f / c.nb_ex_regressors)*((c.known_cost->cost - wc.x)*(c.known_cost->cost - wc.x) - c.avg_loss_regressors);
        c.last_pred_reg = wc.x;
        c.last_correct_cost = c.known_cost->cost;
      }

      cs_ld.costs.push_back(wc);
    }
  }
  else   //this is an example where we can only perform a subset of the actions
  { //in this case generate cost-sensitive example with only allowed actions
    for (auto& cl : ld.costs)
    { COST_SENSITIVE::wclass wc = {0., cl.action, 0., 0.};

      //get cost prediction for this action
      wc.x = CB_ALGS::get_cost_pred<is_learn>(c.scorer, c.known_cost, ec, cl.action, 0);
      if (wc.x < min || (wc.x == min && cl.action < argmin))
      { min = wc.x;
        argmin = cl.action;
      }
      c.pred_scores.costs.push_back(wc);

      if (c.known_cost != nullptr && c.known_cost->action == cl.action)
      { c.nb_ex_regressors++;
        c.avg_loss_regressors += (1.0f / c.nb_ex_regressors)*((c.known_cost->cost - wc.x)*(c.known_cost->cost - wc.x) - c.avg_loss_regressors);
        c.last_pred_reg = wc.x;
        c.last_correct_cost = c.known_cost->cost;
      }

      cs_ld.costs.push_back(wc);
    }
  }

  ec.pred.multiclass = argmin;
}

template <bool is_learn>
void gen_cs_label(cb_to_cs& c, example& ec, COST_SENSITIVE::label& cs_ld, uint32_t action)
{ COST_SENSITIVE::wclass wc = {0., action, 0., 0.};

  //get cost prediction for this action
  wc.x = CB_ALGS::get_cost_pred<is_learn>(c.scorer, c.known_cost, ec, action, c.num_actions);

  c.pred_scores.costs.push_back(wc);
  //add correction if we observed cost for this action and regressor is wrong
  if (c.known_cost != nullptr && c.known_cost->action == action)
  { c.nb_ex_regressors++;
    c.avg_loss_regressors += (1.0f / c.nb_ex_regressors)*((c.known_cost->cost - wc.x)*(c.known_cost->cost - wc.x) - c.avg_loss_regressors);
    c.last_pred_reg = wc.x;
    c.last_correct_cost = c.known_cost->cost;
    wc.x += (c.known_cost->cost - wc.x) / c.known_cost->probability;
  }

  cs_ld.costs.push_back(wc);

}

template <bool is_learn>
void gen_cs_example_dr(cb_to_cs& c, example& ec, CB::label& ld, COST_SENSITIVE::label& cs_ld)
{ //this implements the doubly robust method
  cs_ld.costs.erase();
  c.pred_scores.costs.erase();
  if (ld.costs.size() == 0)//a test example
    for (uint32_t i = 1; i <= c.num_actions; i++)
    { //Explicit declaration for a weak compiler.
      COST_SENSITIVE::wclass c = { FLT_MAX, i, 0., 0. };
      cs_ld.costs.push_back(c);
    }
  else if (ld.costs.size() == 1 && !is_test_label(ld)) //this is a typical example where we can perform all actions
    //in this case generate cost-sensitive example with all actions
    for (uint32_t i = 1; i <= c.num_actions; i++)
    { gen_cs_label<is_learn>(c, ec, cs_ld, i);
    }
  else  //this is an example where we can only perform a subset of the actions
    //in this case generate cost-sensitive example with only allowed actions
    for (auto& cl : ld.costs)
      gen_cs_label<is_learn>(c, ec, cs_ld, cl.action);
}

template <bool is_learn>
void gen_cs_example(cb_to_cs& c, example& ec, CB::label& ld, COST_SENSITIVE::label& cs_ld)
{ switch (c.cb_type)
  { case CB_TYPE_IPS:
      gen_cs_example_ips(c, ld, cs_ld);
      break;
    case CB_TYPE_DM:
      gen_cs_example_dm<is_learn>(c, ec, cs_ld);
      break;
    case CB_TYPE_DR:
      gen_cs_example_dr<is_learn>(c, ec, ld, cs_ld);
      break;
    default:
      THROW("Unknown cb_type specified for contextual bandit learning: " << c.cb_type);
  }
}

void gen_cs_test_example(v_array<example*> examples, COST_SENSITIVE::label& cs_labels);

void gen_cs_example_ips(v_array<example*> examples, COST_SENSITIVE::label& cs_labels);

void gen_cs_example_dm(v_array<example*> examples, COST_SENSITIVE::label& cs_labels);

void gen_cs_example_mtr(cb_to_cs_adf& c, v_array<example*>& ec_seq, COST_SENSITIVE::label& cs_labels);

template <bool is_learn>
void gen_cs_example_dr(cb_to_cs_adf& c, v_array<example*> examples, COST_SENSITIVE::label& cs_labels)
{ //size_t mysize = examples.size();
  c.pred_scores.costs.erase();
  bool shared = CB::ec_is_example_header(*examples[0]);
  int startK = 0;
  if (shared) startK = 1;

  cs_labels.costs.erase();
  for (size_t i = 0; i < examples.size(); i++)
  { if (CB_ALGS::example_is_newline_not_header(*examples[i])) continue;

    COST_SENSITIVE::wclass wc = {0.,0,0.,0.};

    if (c.known_cost.action + startK == i)
    { int known_index = c.known_cost.action;
      c.known_cost.action = 0;
      //get cost prediction for this label
      // num_actions should be 1 effectively.
      // my get_cost_pred function will use 1 for 'index-1+base'
      wc.x = CB_ALGS::get_cost_pred<is_learn>(c.scorer, &(c.known_cost), *(examples[i]), 0, 2);
      c.known_cost.action = known_index;
    }
    else
      wc.x = CB_ALGS::get_cost_pred<is_learn>(c.scorer, nullptr, *(examples[i]), 0, 2);

    if (shared)
      wc.class_index = (uint32_t)i - 1;
    else
      wc.class_index = (uint32_t)i;
    c.pred_scores.costs.push_back(wc); // done

    //add correction if we observed cost for this action and regressor is wrong
    if (c.known_cost.probability != -1 && c.known_cost.action + startK == i)
      wc.x += (c.known_cost.cost - wc.x) / c.known_cost.probability;
    cs_labels.costs.push_back(wc);
  }

  if (shared)//take care of shared examples
  { cs_labels.costs[0].class_index = 0;
    cs_labels.costs[0].x = -FLT_MAX;
  }
}

template <bool is_learn>
void gen_cs_example(cb_to_cs_adf& c, v_array<example*>& ec_seq, COST_SENSITIVE::label& cs_labels)
{ switch (c.cb_type)
  { case CB_TYPE_IPS:
      gen_cs_example_ips(ec_seq, cs_labels);
      break;
    case CB_TYPE_DR:
      gen_cs_example_dr<is_learn>(c, ec_seq, cs_labels);
      break;
    case CB_TYPE_MTR:
      gen_cs_example_mtr(c, ec_seq, cs_labels);
      break;
    default:
      THROW("Unknown cb_type specified for contextual bandit learning: " << c.cb_type);
  }
}

template<bool is_learn>
void call_cs_ldf(LEARNER::base_learner& base, v_array<example*>& examples, v_array<CB::label>& cb_labels,
                 COST_SENSITIVE::label& cs_labels, v_array<COST_SENSITIVE::label>& prepped_cs_labels, uint64_t offset, size_t id = 0)
{ cb_labels.erase();
  if (prepped_cs_labels.size() < cs_labels.costs.size()+1)
  { prepped_cs_labels.resize(cs_labels.costs.size()+1);
    prepped_cs_labels.end() = prepped_cs_labels.end_array;
  }

  // 1st: save cb_label (into mydata) and store cs_label for each example, which will be passed into base.learn.
  size_t index = 0;
  for (example* ec : examples)
  { cb_labels.push_back(ec->l.cb);
    prepped_cs_labels[index].costs.erase();
    if (index != examples.size()-1)
      prepped_cs_labels[index].costs.push_back(cs_labels.costs[index]);
    else
      prepped_cs_labels[index].costs.push_back({FLT_MAX,0,0.,0.});
    ec->l.cs = prepped_cs_labels[index++];
  }

  // 2nd: predict for each ex
  // // call base.predict for each vw exmaple in the sequence
  for (example* ec : examples)
  { uint64_t old_offset = ec->ft_offset;
    ec->ft_offset = offset;
    if (is_learn)
      base.learn(*ec, id);
    else
      base.predict(*ec, id);
    ec->ft_offset = old_offset;

  }
  // 3rd: restore cb_label for each example
  // (**ec).l.cb = array.element.
  size_t i = 0;
  for (example* ec : examples)
    ec->l.cb = cb_labels[i++];
}
}
