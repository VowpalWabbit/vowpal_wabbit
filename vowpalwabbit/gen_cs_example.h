// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include <cfloat>

#include "vw.h"
#include "reductions.h"
#include "cb_algs.h"
#include "vw_exception.h"
#include "scope_exit.h"

namespace GEN_CS
{
struct cb_to_cs
{
  VW::cb_type_t cb_type = VW::cb_type_t::dm;
  uint32_t num_actions = 0;
  COST_SENSITIVE::label pred_scores;
  VW::LEARNER::single_learner* scorer = nullptr;
  float avg_loss_regressors = 0.f;
  size_t nb_ex_regressors = 0;
  float last_pred_reg = 0.f;
  float last_correct_cost = 0.f;

  CB::cb_class known_cost;
};

struct cb_to_cs_adf
{
  VW::cb_type_t cb_type = VW::cb_type_t::dm;

  // for MTR
  uint64_t action_sum = 0;
  uint64_t event_sum = 0;
  uint32_t mtr_example = 0;
  multi_ex mtr_ec_seq;  // shared + the one example.

  // for DR
  COST_SENSITIVE::label pred_scores;
  CB::cb_class known_cost;
  VW::LEARNER::single_learner* scorer = nullptr;
};

float safe_probability(float prob, VW::io::logger& logger);

void gen_cs_example_ips(cb_to_cs& c, const CB::label& ld, COST_SENSITIVE::label& cs_ld, VW::io::logger& logger, float clip_p = 0.f);

template <bool is_learn>
void gen_cs_example_dm(cb_to_cs& c, example& ec, const CB::label& ld, COST_SENSITIVE::label& cs_ld)
{  // this implements the direct estimation method, where costs are directly specified by the learned regressor.

  float min = FLT_MAX;
  uint32_t argmin = 1;
  // generate cost sensitive example
  cs_ld.costs.clear();
  c.pred_scores.costs.clear();

  if (ld.costs.size() == 0 ||
      (ld.costs.size() == 1 &&
          ld.costs[0].cost != FLT_MAX))  // this is a typical example where we can perform all actions
  {                                      // in this case generate cost-sensitive example with all actions
    for (uint32_t i = 1; i <= c.num_actions; i++)
    {
      COST_SENSITIVE::wclass wc = {0., i, 0., 0.};
      // get cost prediction for this action
      wc.x = CB_ALGS::get_cost_pred<is_learn>(c.scorer, c.known_cost, ec, i, 0);
      if (wc.x < min)
      {
        min = wc.x;
        argmin = i;
      }

      c.pred_scores.costs.push_back(wc);

      if (c.known_cost.action == i)
      {
        c.nb_ex_regressors++;
        c.avg_loss_regressors += (1.0f / c.nb_ex_regressors) *
            ((c.known_cost.cost - wc.x) * (c.known_cost.cost - wc.x) - c.avg_loss_regressors);
        c.last_pred_reg = wc.x;
        c.last_correct_cost = c.known_cost.cost;
      }

      cs_ld.costs.push_back(wc);
    }
  }
  else  // this is an example where we can only perform a subset of the actions
  {     // in this case generate cost-sensitive example with only allowed actions
    for (const auto& cl : ld.costs)
    {
      COST_SENSITIVE::wclass wc = {0., cl.action, 0., 0.};

      // get cost prediction for this action
      wc.x = CB_ALGS::get_cost_pred<is_learn>(c.scorer, c.known_cost, ec, cl.action, 0);
      if (wc.x < min || (wc.x == min && cl.action < argmin))
      {
        min = wc.x;
        argmin = cl.action;
      }
      c.pred_scores.costs.push_back(wc);

      if (c.known_cost.action == cl.action)
      {
        c.nb_ex_regressors++;
        c.avg_loss_regressors += (1.0f / c.nb_ex_regressors) *
            ((c.known_cost.cost - wc.x) * (c.known_cost.cost - wc.x) - c.avg_loss_regressors);
        c.last_pred_reg = wc.x;
        c.last_correct_cost = c.known_cost.cost;
      }

      cs_ld.costs.push_back(wc);
    }
  }

  ec.pred.multiclass = argmin;
}

template <bool is_learn>
void gen_cs_label(cb_to_cs& c, example& ec, COST_SENSITIVE::label& cs_ld, uint32_t action, float clip_p = 0.f)
{
  COST_SENSITIVE::wclass wc = {0., action, 0., 0.};

  // get cost prediction for this action
  wc.x = CB_ALGS::get_cost_pred<is_learn>(c.scorer, c.known_cost, ec, action, c.num_actions);

  c.pred_scores.costs.push_back(wc);
  // add correction if we observed cost for this action and regressor is wrong
  if (c.known_cost.action == action)
  {
    c.nb_ex_regressors++;
    c.avg_loss_regressors +=
        (1.0f / c.nb_ex_regressors) * ((c.known_cost.cost - wc.x) * (c.known_cost.cost - wc.x) - c.avg_loss_regressors);
    c.last_pred_reg = wc.x;
    c.last_correct_cost = c.known_cost.cost;
    wc.x += (c.known_cost.cost - wc.x) / std::max(c.known_cost.probability, clip_p);
  }

  cs_ld.costs.push_back(wc);
}

template <bool is_learn>
void gen_cs_example_dr(
    cb_to_cs& c, example& ec, const CB::label& ld, COST_SENSITIVE::label& cs_ld, float /*clip_p*/ = 0.f)
{
  // this implements the doubly robust method
  VW_DBG(ec) << "gen_cs_example_dr:" << is_learn << std::endl;
  cs_ld.costs.clear();
  c.pred_scores.costs.clear();
  if (ld.costs.size() == 0)  // a test example
    for (uint32_t i = 1; i <= c.num_actions; i++)
    {  // Explicit declaration for a weak compiler.
      COST_SENSITIVE::wclass temp = {FLT_MAX, i, 0., 0.};
      cs_ld.costs.push_back(temp);
    }
  else if (ld.costs.size() == 0 || (ld.costs.size() == 1 && ld.costs[0].cost != FLT_MAX))
    // this is a typical example where we can perform all actions
    // in this case generate cost-sensitive example with all actions
    for (uint32_t i = 1; i <= c.num_actions; i++) gen_cs_label<is_learn>(c, ec, cs_ld, i);
  else  // this is an example where we can only perform a subset of the actions
    // in this case generate cost-sensitive example with only allowed actions
    for (auto& cl : ld.costs) gen_cs_label<is_learn>(c, ec, cs_ld, cl.action);
}

template <bool is_learn>
void gen_cs_example(cb_to_cs& c, example& ec, const CB::label& ld, COST_SENSITIVE::label& cs_ld, VW::io::logger& logger)
{
  switch (c.cb_type)
  {
    case VW::cb_type_t::ips:
      gen_cs_example_ips(c, ld, cs_ld, logger);
      break;
    case VW::cb_type_t::dm:
      gen_cs_example_dm<is_learn>(c, ec, ld, cs_ld);
      break;
    case VW::cb_type_t::dr:
      gen_cs_example_dr<is_learn>(c, ec, ld, cs_ld);
      break;
    default:
      THROW("Unknown cb_type specified for contextual bandit learning: " << VW::to_string(c.cb_type));
  }
}

void gen_cs_test_example(const multi_ex& examples, COST_SENSITIVE::label& cs_labels);

void gen_cs_example_ips(const multi_ex& examples, COST_SENSITIVE::label& cs_labels, VW::io::logger& logger, float clip_p = 0.f);

void gen_cs_example_dm(const multi_ex& examples, COST_SENSITIVE::label& cs_labels);

void gen_cs_example_mtr(cb_to_cs_adf& c, multi_ex& ec_seq, COST_SENSITIVE::label& cs_labels);

void gen_cs_example_sm(multi_ex& examples, uint32_t chosen_action, float sign_offset,
    const ACTION_SCORE::action_scores& action_vals, COST_SENSITIVE::label& cs_labels);

template <bool is_learn>
void gen_cs_example_dr(cb_to_cs_adf& c, multi_ex& examples, COST_SENSITIVE::label& cs_labels, float clip_p = 0.f)
{  // size_t mysize = examples.size();
  VW_DBG(*examples[0]) << "gen_cs_example_dr-adf:" << is_learn << std::endl;
  c.pred_scores.costs.clear();

  cs_labels.costs.clear();
  for (size_t i = 0; i < examples.size(); i++)
  {
    if (CB_ALGS::example_is_newline_not_header(*examples[i])) continue;

    COST_SENSITIVE::wclass wc = {0., static_cast<uint32_t>(i), 0., 0.};

    if (c.known_cost.action == i)
    {
      int known_index = c.known_cost.action;
      c.known_cost.action = 0;
      // get cost prediction for this label
      // num_actions should be 1 effectively.
      // my get_cost_pred function will use 1 for 'index-1+base'
      wc.x = CB_ALGS::get_cost_pred<is_learn>(c.scorer, c.known_cost, *(examples[i]), 0, 2);
      c.known_cost.action = known_index;
    }
    else
      wc.x = CB_ALGS::get_cost_pred<is_learn>(c.scorer, CB::cb_class{}, *(examples[i]), 0, 2);

    c.pred_scores.costs.push_back(wc);  // done

    // add correction if we observed cost for this action and regressor is wrong
    if (c.known_cost.probability != -1 && c.known_cost.action == i)
      wc.x += (c.known_cost.cost - wc.x) / std::max(c.known_cost.probability, clip_p);
    cs_labels.costs.push_back(wc);
  }
}

template <bool is_learn>
void gen_cs_example(cb_to_cs_adf& c, multi_ex& ec_seq, COST_SENSITIVE::label& cs_labels, VW::io::logger& logger)
{
  VW_DBG(*ec_seq[0]) << "gen_cs_example:" << is_learn << std::endl;
  switch (c.cb_type)
  {
    case VW::cb_type_t::ips:
      gen_cs_example_ips(ec_seq, cs_labels, logger);
      break;
    case VW::cb_type_t::dr:
      gen_cs_example_dr<is_learn>(c, ec_seq, cs_labels);
      break;
    case VW::cb_type_t::mtr:
      gen_cs_example_mtr(c, ec_seq, cs_labels);
      break;
    default:
      THROW("Unknown cb_type specified for contextual bandit learning: " << VW::to_string(c.cb_type));
  }
}

void cs_prep_labels(multi_ex& examples, std::vector<CB::label>& cb_labels, COST_SENSITIVE::label& cs_labels,
    std::vector<COST_SENSITIVE::label>& prepped_cs_labels, uint64_t offset);

template <bool is_learn>
void cs_ldf_learn_or_predict(VW::LEARNER::multi_learner& base, multi_ex& examples, std::vector<CB::label>& cb_labels,
    COST_SENSITIVE::label& cs_labels, std::vector<COST_SENSITIVE::label>& prepped_cs_labels, bool predict_first,
    uint64_t offset, size_t id = 0)
{
  VW_DBG(*examples[0]) << "cs_ldf_" << (is_learn ? "<learn>" : "<predict>") << ": ex=" << examples[0]->example_counter
                       << ", offset=" << offset << ", id=" << id << std::endl;

  cs_prep_labels(examples, cb_labels, cs_labels, prepped_cs_labels, offset);

  // 1st: save cb_label (into mydata) and store cs_label for each example, which will be passed into base.learn.
  // also save offsets
  uint64_t saved_offset = examples[0]->ft_offset;

  // Guard example state restore against throws
  auto restore_guard = VW::scope_exit([&cb_labels, &prepped_cs_labels, saved_offset, &examples] {
    // 3rd: restore cb_label for each example
    // (**ec).l.cb = array.element.
    // and restore offsets
    for (size_t i = 0; i < examples.size(); ++i)
    {
      prepped_cs_labels[i] = std::move(examples[i]->l.cs);
      examples[i]->l.cs.costs.clear();
      examples[i]->l.cb = std::move(cb_labels[i]);
      examples[i]->ft_offset = saved_offset;
    }
  });

  if (is_learn)
  {
    if (predict_first) { base.predict(examples, static_cast<int32_t>(id)); }
    base.learn(examples, static_cast<int32_t>(id));
  }
  else
    base.predict(examples, static_cast<int32_t>(id));
}

}  // namespace GEN_CS
