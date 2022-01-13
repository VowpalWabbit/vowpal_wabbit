// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cfloat>

#include "vw.h"
#include "reductions.h"
#include "vw_exception.h"
#include "gen_cs_example.h"

#include "io/logger.h"

namespace GEN_CS
{
using namespace VW::LEARNER;
using namespace CB_ALGS;
using namespace CB;

float safe_probability(float prob, VW::io::logger& logger)
{
  if (prob <= 0.)
  {
    logger.warn(
        "Probability {} is not possible, replacing with 1e-3. There seems to be something wrong with the dataset.",
        prob);
    return 1e-3f;
  }
  else
    return prob;
}

// Multiline version
void gen_cs_example_ips(
    const multi_ex& examples, COST_SENSITIVE::label& cs_labels, VW::io::logger& logger, float clip_p)
{
  cs_labels.costs.clear();
  for (uint32_t i = 0; i < examples.size(); i++)
  {
    const CB::label& ld = examples[i]->l.cb;

    COST_SENSITIVE::wclass wc = {0., i, 0., 0.};
    if (ld.costs.size() == 1 && ld.costs[0].cost != FLT_MAX)
      wc.x = ld.costs[0].cost / safe_probability(std::max(ld.costs[0].probability, clip_p), logger);
    cs_labels.costs.push_back(wc);
  }
}

// Multiline version
void gen_cs_example_dm(const multi_ex& examples, COST_SENSITIVE::label& cs_labels)
{
  cs_labels.costs.clear();
  for (uint32_t i = 0; i < examples.size(); i++)
  {
    const CB::label& ld = examples[i]->l.cb;

    COST_SENSITIVE::wclass wc = {0., i, 0., 0.};
    if (ld.costs.size() == 1 && ld.costs[0].cost != FLT_MAX) wc.x = ld.costs[0].cost;
    cs_labels.costs.push_back(wc);
  }
}

// Multiline version
void gen_cs_test_example(const multi_ex& examples, COST_SENSITIVE::label& cs_labels)
{
  cs_labels.costs.clear();
  for (uint32_t i = 0; i < examples.size(); i++)
  {
    COST_SENSITIVE::wclass wc = {FLT_MAX, i, 0., 0.};
    cs_labels.costs.push_back(wc);
  }
}

// single line version
void gen_cs_example_ips(
    cb_to_cs& c, const CB::label& ld, COST_SENSITIVE::label& cs_ld, VW::io::logger& logger, float clip_p)
{
  // this implements the inverse propensity score method, where cost are importance weighted by the probability of the
  // chosen action generate cost-sensitive example
  cs_ld.costs.clear();
  if (ld.costs.size() == 0 || (ld.costs.size() == 1 && ld.costs[0].cost != FLT_MAX))
  // this is a typical example where we can perform all actions
  {
    // in this case generate cost-sensitive example with all actions
    for (uint32_t i = 1; i <= c.num_actions; i++)
    {
      COST_SENSITIVE::wclass wc = {0., i, 0., 0.};
      if (i == c.known_cost.action)
      {
        // use importance weighted cost for observed action, 0 otherwise
        wc.x = c.known_cost.cost / safe_probability(std::max(c.known_cost.probability, clip_p), logger);

        // ips can be thought as the doubly robust method with a fixed regressor that predicts 0 costs for everything
        // update the loss of this regressor
        c.nb_ex_regressors++;
        c.avg_loss_regressors +=
            (1.0f / c.nb_ex_regressors) * ((c.known_cost.cost) * (c.known_cost.cost) - c.avg_loss_regressors);
        c.last_pred_reg = 0;
        c.last_correct_cost = c.known_cost.cost;
      }

      cs_ld.costs.push_back(wc);
    }
  }
  else  // this is an example where we can only perform a subset of the actions
  {
    // in this case generate cost-sensitive example with only allowed actions
    for (const auto& cl : ld.costs)
    {
      COST_SENSITIVE::wclass wc = {0., cl.action, 0., 0.};
      if (cl.action == c.known_cost.action)
      {
        // use importance weighted cost for observed action, 0 otherwise
        wc.x = c.known_cost.cost / safe_probability(std::max(c.known_cost.probability, clip_p), logger);

        // ips can be thought as the doubly robust method with a fixed regressor that predicts 0 costs for everything
        // update the loss of this regressor
        c.nb_ex_regressors++;
        c.avg_loss_regressors +=
            (1.0f / c.nb_ex_regressors) * ((c.known_cost.cost) * (c.known_cost.cost) - c.avg_loss_regressors);
        c.last_pred_reg = 0;
        c.last_correct_cost = c.known_cost.cost;
      }

      cs_ld.costs.push_back(wc);
    }
  }
}

void gen_cs_example_mtr(cb_to_cs_adf& c, multi_ex& ec_seq, COST_SENSITIVE::label& cs_labels)
{
  c.action_sum += ec_seq.size();
  c.event_sum++;

  c.mtr_ec_seq.clear();
  cs_labels.costs.clear();
  for (size_t i = 0; i < ec_seq.size(); i++)
  {
    CB::label& ld = ec_seq[i]->l.cb;

    COST_SENSITIVE::wclass wc = {0, 0, 0, 0};

    if (ld.costs.size() == 1 && ld.costs[0].cost != FLT_MAX)
    {
      wc.x = ld.costs[0].cost;
      c.mtr_example = static_cast<uint32_t>(i);
      c.mtr_ec_seq.push_back(ec_seq[i]);
      cs_labels.costs.push_back(wc);
      break;
    }
  }
}

void gen_cs_example_sm(multi_ex&, uint32_t chosen_action, float sign_offset,
    const ACTION_SCORE::action_scores& action_vals, COST_SENSITIVE::label& cs_labels)
{
  cs_labels.costs.clear();
  for (uint32_t i = 0; i < action_vals.size(); i++)
  {
    uint32_t current_action = action_vals[i].action;
    COST_SENSITIVE::wclass wc = {0., current_action, 0., 0.};

    if (current_action == chosen_action)
      wc.x = action_vals[i].score + sign_offset;
    else
      wc.x = action_vals[i].score - sign_offset;

    // TODO: This clipping is conceptually unnecessary because the example weight for this instance should be close to
    // 0.
    if (wc.x > 100.) wc.x = 100.0;
    if (wc.x < -100.) wc.x = -100.0;

    cs_labels.costs.push_back(wc);
  }
}

void cs_prep_labels(multi_ex& examples, std::vector<CB::label>& cb_labels, COST_SENSITIVE::label& cs_labels,
    std::vector<COST_SENSITIVE::label>& prepped_cs_labels, uint64_t offset)
{
  cb_labels.clear();
  if (prepped_cs_labels.size() < cs_labels.costs.size() + 1) { prepped_cs_labels.resize(cs_labels.costs.size() + 1); }

  size_t index = 0;
  for (auto ec : examples)
  {
    cb_labels.emplace_back(std::move(ec->l.cb));
    prepped_cs_labels[index].costs.clear();
    prepped_cs_labels[index].costs.push_back(cs_labels.costs[index]);
    ec->l.cs = std::move(prepped_cs_labels[index++]);
    ec->ft_offset = offset;
  }
}

}  // namespace GEN_CS
