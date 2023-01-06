// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/gen_cs_example.h"

#include "vw/common/vw_exception.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"

#include <cfloat>

float VW::details::safe_probability(float prob, VW::io::logger& logger)
{
  if (prob <= 0.)
  {
    logger.out_warn(
        "Probability {} is not possible, replacing with 1e-3. There seems to be something wrong with the dataset.",
        prob);
    return 1e-3f;
  }
  else { return prob; }
}

// Multiline version
void VW::details::gen_cs_example_ips(
    const VW::multi_ex& examples, VW::cs_label& cs_labels, VW::io::logger& logger, float clip_p)
{
  cs_labels.costs.clear();
  for (uint32_t i = 0; i < examples.size(); i++)
  {
    const VW::cb_label& ld = examples[i]->l.cb;

    VW::cs_class wc = {0., i, 0., 0.};
    if (ld.costs.size() == 1 && ld.costs[0].cost != FLT_MAX)
    {
      wc.x = ld.costs[0].cost / safe_probability(std::max(ld.costs[0].probability, clip_p), logger);
    }
    cs_labels.costs.push_back(wc);
  }
}

// Multiline version
void VW::details::gen_cs_example_dm(const VW::multi_ex& examples, VW::cs_label& cs_labels)
{
  cs_labels.costs.clear();
  for (uint32_t i = 0; i < examples.size(); i++)
  {
    const VW::cb_label& ld = examples[i]->l.cb;

    VW::cs_class wc = {0., i, 0., 0.};
    if (ld.costs.size() == 1 && ld.costs[0].cost != FLT_MAX) { wc.x = ld.costs[0].cost; }
    cs_labels.costs.push_back(wc);
  }
}

// Multiline version
void VW::details::gen_cs_test_example(const VW::multi_ex& examples, VW::cs_label& cs_labels)
{
  cs_labels.costs.clear();
  for (uint32_t i = 0; i < examples.size(); i++)
  {
    VW::cs_class wc = {FLT_MAX, i, 0., 0.};
    cs_labels.costs.push_back(wc);
  }
}

// single line version
void VW::details::gen_cs_example_ips(
    cb_to_cs& c, const VW::cb_label& ld, VW::cs_label& cs_ld, VW::io::logger& logger, float clip_p)
{
  // this implements the inverse propensity score method, where cost are importance weighted by the probability of the
  // chosen action generate cost-sensitive example
  cs_ld.costs.clear();
  if (ld.costs.empty() || (ld.costs.size() == 1 && ld.costs[0].cost != FLT_MAX))
  // this is a typical example where we can perform all actions
  {
    // in this case generate cost-sensitive example with all actions
    for (uint32_t i = 1; i <= c.num_actions; i++)
    {
      VW::cs_class wc = {0., i, 0., 0.};
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
      VW::cs_class wc = {0., cl.action, 0., 0.};
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

void VW::details::gen_cs_example_mtr(cb_to_cs_adf& c, VW::multi_ex& ec_seq, VW::cs_label& cs_labels)
{
  c.action_sum += ec_seq.size();
  c.event_sum++;

  c.mtr_ec_seq.clear();
  cs_labels.costs.clear();
  for (size_t i = 0; i < ec_seq.size(); i++)
  {
    VW::cb_label& ld = ec_seq[i]->l.cb;

    VW::cs_class wc = {0, 0, 0, 0};

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

void VW::details::gen_cs_example_sm(VW::multi_ex&, uint32_t chosen_action, float sign_offset,
    const VW::action_scores& action_vals, VW::cs_label& cs_labels)
{
  cs_labels.costs.clear();
  for (const auto& action_val : action_vals)
  {
    uint32_t current_action = action_val.action;
    VW::cs_class wc = {0., current_action, 0., 0.};

    if (current_action == chosen_action) { wc.x = action_val.score + sign_offset; }
    else { wc.x = action_val.score - sign_offset; }

    // TODO: This clipping is conceptually unnecessary because the example weight for this instance should be close to
    // 0.
    if (wc.x > 100.) { wc.x = 100.0; }
    if (wc.x < -100.) { wc.x = -100.0; }

    cs_labels.costs.push_back(wc);
  }
}

void VW::details::cs_prep_labels(VW::multi_ex& examples, std::vector<VW::cb_label>& cb_labels, VW::cs_label& cs_labels,
    std::vector<VW::cs_label>& prepped_cs_labels, uint64_t offset)
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
