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

namespace GEN_CS{

struct cb_to_cs
{
  size_t cb_type;
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
{
  size_t cb_type;

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
	{
	  COST_SENSITIVE::wclass wc = {0., i, 0., 0.};
	  //get cost prediction for this action
	  wc.x = CB_ALGS::get_cost_pred<is_learn>(c.scorer, c.known_cost, ec, i, 0);
	  if (wc.x < min)
	    {
	      min = wc.x;
	      argmin = i;
	    }

	  c.pred_scores.costs.push_back(wc);

	  if (c.known_cost != nullptr && c.known_cost->action == i)
	    {
	      c.nb_ex_regressors++;
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
	{
	  COST_SENSITIVE::wclass wc = {0., cl.action, 0., 0.};

	  //get cost prediction for this action
	  wc.x = CB_ALGS::get_cost_pred<is_learn>(c.scorer, c.known_cost, ec, cl.action, 0);
	  if (wc.x < min || (wc.x == min && cl.action < argmin))
	    {
	      min = wc.x;
	      argmin = cl.action;
	    }
	  c.pred_scores.costs.push_back(wc);

	  if (c.known_cost != nullptr && c.known_cost->action == cl.action)
	    {
	      c.nb_ex_regressors++;
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
{
  COST_SENSITIVE::wclass wc = {0., action, 0., 0.};

  //get cost prediction for this action
  wc.x = CB_ALGS::get_cost_pred<is_learn>(c.scorer, c.known_cost, ec, action, c.num_actions);

  c.pred_scores.costs.push_back(wc);
  //add correction if we observed cost for this action and regressor is wrong
  if (c.known_cost != nullptr && c.known_cost->action == action)
    {
      c.nb_ex_regressors++;
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
    for (uint32_t i = 1; i <= c.num_actions; i++) {
      gen_cs_label<is_learn>(c, ec, cs_ld, i);
    }
  else  //this is an example where we can only perform a subset of the actions
    //in this case generate cost-sensitive example with only allowed actions
    for (auto& cl : ld.costs)
      gen_cs_label<is_learn>(c, ec, cs_ld, cl.action);
}

template <bool is_learn>
void gen_cs_example(cb_to_cs& c, example& ec, CB::label& ld, COST_SENSITIVE::label& cs_ld)
{
  switch (c.cb_type)
    {
    case CB_TYPE_IPS:
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

 void gen_cs_example_ips(v_array<example*> examples, v_array<COST_SENSITIVE::label>& cs_labels);

 void gen_cs_example_mtr(cb_to_cs_adf& c, v_array<example*>& ec_seq, v_array<COST_SENSITIVE::label>& cs_labels);

 void gen_cs_example_dr(cb_to_cs_adf& c, v_array<example*> ec_seq, v_array<COST_SENSITIVE::label>& cs_labels);

 template <bool is_learn>
   void gen_cs_example(cb_to_cs_adf& c, v_array<example*>& ec_seq, v_array<COST_SENSITIVE::label>& cs_labels)
   {
     switch (c.cb_type)
       {
       case CB_TYPE_IPS:
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
}
