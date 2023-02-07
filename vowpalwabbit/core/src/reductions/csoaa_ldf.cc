// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "vw/core/reductions/csoaa_ldf.h"

#include "vw/common/vw_exception.h"
#include "vw/config/options.h"
#include "vw/core/constant.h"
#include "vw/core/correctedMath.h"
#include "vw/core/cost_sensitive.h"
#include "vw/core/label_dictionary.h"
#include "vw/core/learner.h"
#include "vw/core/loss_functions.h"
#include "vw/core/multi_ex.h"
#include "vw/core/prediction_type.h"
#include "vw/core/print_utils.h"
#include "vw/core/reductions/gd.h"  // VW::foreach_feature() needed in subtract_example()
#include "vw/core/scope_exit.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/v_array.h"
#include "vw/core/vw.h"
#include "vw/core/vw_fwd.h"
#include "vw/io/logger.h"

#include <algorithm>
#include <cerrno>
#include <cfloat>
#include <cmath>

using namespace VW::LEARNER;
using namespace VW::config;

#undef VW_DEBUG_LOG
#define VW_DEBUG_LOG vw_dbg::CSOAA_LDF

namespace
{
// TODO: passthrough for ldf
class ldf
{
public:
  VW::details::label_feature_map label_features;

  bool is_wap = false;
  bool treat_as_classifier = false;
  bool is_probabilities = false;
  VW::workspace* all = nullptr;

  bool rank = false;
  VW::action_scores a_s;
  uint64_t ft_offset = 0;

  std::vector<VW::action_scores> stored_preds;
};

inline bool cmp_wclass_ptr(const VW::cs_class* a, const VW::cs_class* b) { return a->x < b->x; }

void compute_wap_values(std::vector<VW::cs_class*> costs)
{
  std::sort(costs.begin(), costs.end(), cmp_wclass_ptr);
  costs[0]->wap_value = 0.;
  for (size_t i = 1; i < costs.size(); i++)
  {
    costs[i]->wap_value = costs[i - 1]->wap_value + (costs[i]->x - costs[i - 1]->x) / static_cast<float>(i);
  }
}

// Substract a given feature from example ec.
// Rather than finding the corresponding namespace and feature in ec,
// add a new feature with opposite value (but same index) to ec to a special VW::details::WAP_LDF_NAMESPACE.
// This is faster and allows fast undo in unsubtract_example().
void subtract_feature(VW::example& ec, float feature_value_x, uint64_t weight_index)
{
  ec.feature_space[VW::details::WAP_LDF_NAMESPACE].push_back(
      -feature_value_x, weight_index, VW::details::WAP_LDF_NAMESPACE);
}

// Iterate over all features of ecsub including quadratic and cubic features and subtract them from ec.
void subtract_example(VW::workspace& all, VW::example* ec, VW::example* ecsub)
{
  auto& wap_fs = ec->feature_space[VW::details::WAP_LDF_NAMESPACE];
  wap_fs.sum_feat_sq = 0;
  VW::foreach_feature<VW::example&, uint64_t, subtract_feature>(all, *ecsub, *ec);
  ec->indices.push_back(VW::details::WAP_LDF_NAMESPACE);
  ec->num_features += wap_fs.size();
  ec->reset_total_sum_feat_sq();
}

void unsubtract_example(VW::example* ec, VW::io::logger& logger)
{
  if (ec->indices.empty())
  {
    logger.err_error("Internal error (bug): trying to unsubtract_example, but there are no namespaces");
    return;
  }

  if (ec->indices.back() != VW::details::WAP_LDF_NAMESPACE)
  {
    logger.err_error(
        "Internal error (bug): trying to unsubtract_example, but either it wasn't added, or something was added "
        "after and not removed");
    return;
  }

  auto& fs = ec->feature_space[VW::details::WAP_LDF_NAMESPACE];
  ec->num_features -= fs.size();
  ec->reset_total_sum_feat_sq();
  fs.clear();
  ec->indices.pop_back();
}

void make_single_prediction(ldf& data, learner& base, VW::example& ec)
{
  uint64_t old_offset = ec.ft_offset;

  VW::details::append_example_namespace_from_memory(data.label_features, ec, ec.l.cs.costs[0].class_index);

  auto restore_guard = VW::scope_exit(
      [&data, old_offset, &ec]
      {
        ec.ft_offset = old_offset;
        // WARNING: Access of label information when making prediction is
        // problematic.
        ec.l.cs.costs[0].partial_prediction = ec.partial_prediction;
        // WARNING: Access of label information when making prediction is
        // problematic.
        VW::details::truncate_example_namespace_from_memory(data.label_features, ec, ec.l.cs.costs[0].class_index);
      });

  ec.l.simple = VW::simple_label{FLT_MAX};
  ec.ex_reduction_features.template get<VW::simple_label_reduction_features>().reset_to_default();

  ec.ft_offset = data.ft_offset;
  base.predict(ec);  // make a prediction
}

bool test_ldf_sequence(const VW::multi_ex& ec_seq, VW::io::logger& logger)
{
  bool is_test;
  if (ec_seq.empty()) { is_test = true; }
  else { is_test = ec_seq[0]->l.cs.is_test_label(); }
  for (const auto& ec : ec_seq)
  {
    // Each sub-example must have just one cost
    assert(ec->l.cs.costs.size() == 1);

    if (ec->l.cs.is_test_label() != is_test)
    {
      is_test = true;
      logger.err_warn("ldf example has mix of train/test data; assuming test");
    }
  }
  return is_test;
}

void do_actual_learning_wap(ldf& data, learner& base, VW::multi_ex& ec_seq)
{
  VW_DBG(ec_seq) << "do_actual_learning_wap()" << std::endl;

  size_t num_classes = ec_seq.size();
  std::vector<VW::cs_class*> all_costs;
  for (const auto& example : ec_seq) { all_costs.push_back(&example->l.cs.costs[0]); }
  compute_wap_values(all_costs);

  for (size_t k1 = 0; k1 < num_classes; k1++)
  {
    VW::example* ec1 = ec_seq[k1];

    // save original variables
    VW::cs_label save_cs_label = ec1->l.cs;
    auto& simple_lbl = ec1->l.simple;
    auto& simple_red_features = ec1->ex_reduction_features.template get<VW::simple_label_reduction_features>();

    auto costs1 = save_cs_label.costs;
    if (costs1[0].class_index == static_cast<uint32_t>(-1)) { continue; }

    VW::details::append_example_namespace_from_memory(data.label_features, *ec1, costs1[0].class_index);

    // Guard example state restore against throws
    auto restore_guard = VW::scope_exit(
        [&data, &save_cs_label, &costs1, &ec1]
        {
          VW::details::truncate_example_namespace_from_memory(data.label_features, *ec1, costs1[0].class_index);

          // restore original cost-sensitive label, sum of importance weights
          ec1->l.cs = save_cs_label;
        });

    for (size_t k2 = k1 + 1; k2 < num_classes; k2++)
    {
      VW::example* ec2 = ec_seq[k2];
      auto costs2 = ec2->l.cs.costs;

      if (costs2[0].class_index == static_cast<uint32_t>(-1)) { continue; }
      float value_diff = std::fabs(costs2[0].wap_value - costs1[0].wap_value);
      // float value_diff = fabs(costs2[0].x - costs1[0].x);
      if (value_diff < 1e-6) { continue; }

      // Prepare example for learning
      VW::details::append_example_namespace_from_memory(data.label_features, *ec2, costs2[0].class_index);

      // learn
      float old_weight = ec1->weight;
      uint64_t old_offset = ec1->ft_offset;
      simple_red_features.initial = 0.;
      simple_lbl.label = (costs1[0].x < costs2[0].x) ? -1.0f : 1.0f;
      ec1->weight = value_diff;
      ec1->partial_prediction = 0.;
      subtract_example(*data.all, ec1, ec2);
      ec1->ft_offset = data.ft_offset;

      // Guard inner example state restore against throws
      auto restore_guard_inner = VW::scope_exit(
          [&data, old_offset, old_weight, &costs2, &ec2, &ec1]
          {
            ec1->ft_offset = old_offset;
            ec1->weight = old_weight;
            unsubtract_example(ec1, data.all->logger);
            VW::details::truncate_example_namespace_from_memory(data.label_features, *ec2, costs2[0].class_index);
          });

      base.learn(*ec1);
    }
    // TODO: What about partial_prediction? See do_actual_learning_oaa.
  }
}

void do_actual_learning_oaa(ldf& data, learner& base, VW::multi_ex& ec_seq)
{
  VW_DBG(ec_seq) << "do_actual_learning_oaa()" << std::endl;

  float min_cost = FLT_MAX;
  float max_cost = -FLT_MAX;

  for (const auto& example : ec_seq)
  {
    float ec_cost = example->l.cs.costs[0].x;
    if (ec_cost < min_cost) { min_cost = ec_cost; }
    if (ec_cost > max_cost) { max_cost = ec_cost; }
  }

  for (const auto& ec : ec_seq)
  {
    // save original variables
    VW::cs_label save_cs_label = std::move(ec->l.cs);
    const auto& costs = save_cs_label.costs;

    // build example for the base learner
    VW::simple_label simple_lbl;

    float old_weight = ec->weight;
    if (!data.treat_as_classifier)
    {  // treat like regression
      simple_lbl.label = costs[0].x;
    }
    else  // treat like classification
    {
      if (costs[0].x <= min_cost)
      {
        simple_lbl.label = -1.;
        ec->weight = old_weight * (max_cost - min_cost);
      }
      else
      {
        simple_lbl.label = 1.;
        ec->weight = old_weight * (costs[0].x - min_cost);
      }
    }
    auto& simple_red_features = ec->ex_reduction_features.template get<VW::simple_label_reduction_features>();
    simple_red_features.initial = 0.;
    ec->l.simple = simple_lbl;

    // Prepare examples for learning
    VW::details::append_example_namespace_from_memory(data.label_features, *ec, costs[0].class_index);
    uint64_t old_offset = ec->ft_offset;
    ec->ft_offset = data.ft_offset;

    // Guard example state restore against throws
    auto restore_guard = VW::scope_exit(
        [&save_cs_label, &data, &costs, old_offset, old_weight, &ec]
        {
          ec->ft_offset = old_offset;
          VW::details::truncate_example_namespace_from_memory(data.label_features, *ec, costs[0].class_index);
          ec->weight = old_weight;
          ec->partial_prediction = costs[0].partial_prediction;
          // restore original cost-sensitive label, sum of importance weights and partial_prediction
          ec->l.cs = std::move(save_cs_label);
        });

    base.learn(*ec);
  }
}

/*
 * 1) process all labels at first
 * 2) verify no labels in the middle of data
 * 3) learn_or_predict(data) with rest
 */
void learn_csoaa_ldf(ldf& data, learner& base, VW::multi_ex& ec_seq_all)
{
  if (ec_seq_all.empty())
  {
    return;  // nothing to do
  }

  data.ft_offset = ec_seq_all[0]->ft_offset;

  /////////////////////// learn
  if (!test_ldf_sequence(ec_seq_all, data.all->logger))
  {
    if (data.is_wap) { do_actual_learning_wap(data, base, ec_seq_all); }
    else { do_actual_learning_oaa(data, base, ec_seq_all); }
  }
}

void convert_to_probabilities(const VW::multi_ex& ec_seq, VW::v_array<float>& probabilities)
{
  probabilities.clear();
  probabilities.reserve(ec_seq.size());
  float sum_prob = 0;
  for (const auto* example : ec_seq)
  {
    // probability(correct_class) = 1 / (1+exp(-score)), where score is higher
    // for better classes,
    // but partial_prediction is lower for better classes (we are predicting the
    // cost),
    // so we need to take score = -partial_prediction,
    // thus probability(correct_class) = 1 / (1+exp(-(-partial_prediction)))
    float prob = 1.f / (1.f + VW::details::correctedExp(example->partial_prediction));
    probabilities.push_back(prob);
    sum_prob += prob;
  }
  // make sure that the probabilities sum up (exactly) to one
  for (auto& prob : probabilities) { prob /= sum_prob; }
}

/*
 * 1) process all labels at first
 * 2) verify no labels in the middle of data
 * 3) learn_or_predict(data) with rest
 */
void predict_csoaa_ldf(ldf& data, learner& base, VW::multi_ex& ec_seq_all)
{
  if (ec_seq_all.empty())
  {
    return;  // nothing to do
  }

  data.ft_offset = ec_seq_all[0]->ft_offset;

  uint32_t num_classes = static_cast<uint32_t>(ec_seq_all.size());
  // Predicted class as index of input examples.
  uint32_t predicted_class = 0;

  auto restore_guard =
      VW::scope_exit([&ec_seq_all, &predicted_class] { ec_seq_all[0]->pred.multiclass = predicted_class; });

  /////////////////////// do prediction
  float min_score = FLT_MAX;
  for (uint32_t k = 0; k < num_classes; k++)
  {
    VW::example* ec = ec_seq_all[k];
    make_single_prediction(data, base, *ec);
    if (ec->partial_prediction < min_score)
    {
      min_score = ec->partial_prediction;
      predicted_class = ec->l.cs.costs[0].class_index;
    }
  }
}

void predict_csoaa_ldf_probabilities(ldf& data, learner& base, VW::multi_ex& ec_seq_all)
{
  if (ec_seq_all.empty())
  {
    return;  // nothing to do
  }

  data.ft_offset = ec_seq_all[0]->ft_offset;
  auto restore_guard =
      VW::scope_exit([&ec_seq_all] { convert_to_probabilities(ec_seq_all, ec_seq_all[0]->pred.scalars); });

  /////////////////////// do prediction
  for (auto* ec : ec_seq_all) { make_single_prediction(data, base, *ec); }
}

/*
 * 1) process all labels at first
 * 2) verify no labels in the middle of data
 * 3) learn_or_predict(data) with rest
 */
void predict_csoaa_ldf_rank(ldf& data, learner& base, VW::multi_ex& ec_seq_all)
{
  data.ft_offset = ec_seq_all[0]->ft_offset;
  if (ec_seq_all.empty())
  {
    return;  // nothing more to do
  }

  uint32_t num_classes = static_cast<uint32_t>(ec_seq_all.size());

  /////////////////////// do prediction
  data.a_s.clear();
  data.stored_preds.clear();

  auto restore_guard = VW::scope_exit(
      [&data, &ec_seq_all, num_classes]
      {
        std::sort(data.a_s.begin(), data.a_s.end());

        data.stored_preds[0].clear();
        for (size_t k = 0; k < num_classes; k++)
        {
          ec_seq_all[k]->pred.a_s = std::move(data.stored_preds[k]);
          ec_seq_all[0]->pred.a_s.push_back(data.a_s[k]);
        }
      });

  for (uint32_t k = 0; k < num_classes; k++)
  {
    VW::example* ec = ec_seq_all[k];
    data.stored_preds.emplace_back(std::move(ec->pred.a_s));
    make_single_prediction(data, base, *ec);
    VW::action_score s;
    s.score = ec->partial_prediction;
    s.action = ec->l.cs.costs[0].class_index;
    data.a_s.push_back(s);
  }
}

void csoaa_ldf_multiclass_printline(
    VW::workspace& all, VW::io::writer* output, const VW::multi_ex& ec_seq, VW::io::logger& logger)
{
  const auto predicted_class = ec_seq[0]->pred.multiclass;
  for (const auto* ec : ec_seq)
  {
    if (VW::is_cs_example_header(*ec)) { continue; }

    // predicted example;
    bool predicted_example = false;
    for (const auto cost : ec->l.cs.costs)
    {
      if (cost.class_index == predicted_class)
      {
        predicted_example = true;
        break;
      }
    }

    if (predicted_example) { all.print_by_ref(output, predicted_class, 0, ec->tag, logger); }
    else { all.print_by_ref(output, 0, 0, ec->tag, logger); }
  }
}

void csoaa_ldf_print_raw(VW::workspace& all, VW::io::writer* output, const VW::multi_ex& ec_seq, VW::io::logger& logger)
{
  for (const auto* ec : ec_seq)
  {
    const auto& costs = ec->l.cs.costs;
    std::string output_string;
    std::stringstream output_string_stream(output_string);
    for (size_t i = 0; i < costs.size(); i++)
    {
      if (i > 0) { output_string_stream << ' '; }
      output_string_stream << costs[i].class_index << ':' << costs[i].partial_prediction;
    }
    // outputStringStream << std::endl;
    all.print_text_by_ref(output, output_string_stream.str(), ec->tag, logger);
  }
}

size_t cs_count_features(const VW::multi_ex& ec_seq)
{
  size_t num_features = 0;
  for (const auto* ec : ec_seq)
  {
    if (VW::is_cs_example_header(*ec))
    {
      num_features +=
          (ec_seq.size() - 1) * (ec->get_num_features() - ec->feature_space[VW::details::CONSTANT_NAMESPACE].size());
    }
    else { num_features += ec->get_num_features(); }
  }
  return num_features;
}

const VW::example* get_example_with_labelled_class(uint32_t class_to_find, const VW::multi_ex& ec_seq)
{
  for (const auto* ec : ec_seq)
  {
    assert(!VW::example_is_newline(*ec) && "newline examples should be filtered out before calling this function");
    if (VW::is_cs_example_header(*ec)) { continue; }
    for (const auto& cost : ec->l.cs.costs)
    {
      if (cost.class_index == class_to_find) { return ec; }
    }
  }
  return nullptr;
}

void end_pass(ldf&) {}

void update_stats_csoaa_ldf_rank(const VW::workspace& /* all */, VW::shared_data& sd, const ldf& /* data */,
    const VW::multi_ex& ec_seq, VW::io::logger& logger)
{
  if (test_ldf_sequence(ec_seq, logger)) { sd.weighted_unlabeled_examples += ec_seq[0]->weight; }
  else { sd.weighted_labeled_examples += ec_seq[0]->weight; }
  sd.example_number++;
  const auto& head_ec = *ec_seq[0];

  size_t num_features = cs_count_features(ec_seq);
  sd.total_features += num_features;

  const VW::v_array<VW::action_score>& preds = head_ec.pred.a_s;

  const auto* predicted_ec = get_example_with_labelled_class(preds[0].action, ec_seq);
  if (predicted_ec != nullptr)
  {
    const auto loss = predicted_ec->l.cs.costs[0].x;
    assert(loss >= 0);
    sd.sum_loss += loss;
    sd.sum_loss_since_last_dump += loss;
  }
}

void output_example_prediction_csoaa_ldf_rank(
    VW::workspace& all, const ldf& /* data */, const VW::multi_ex& ec_seq, VW::io::logger& logger)
{
  const auto& head_ec = *ec_seq[0];
  for (auto& sink : all.final_prediction_sink)
  {
    VW::details::print_action_score(sink.get(), head_ec.pred.a_s, head_ec.tag, logger);
  }
  if (all.raw_prediction != nullptr) { csoaa_ldf_print_raw(all, all.raw_prediction.get(), ec_seq, logger); }
  VW::details::global_print_newline(all.final_prediction_sink, logger);
}

void print_update_csoaa_ldf_rank(VW::workspace& all, VW::shared_data& /* sd */, const ldf& /* data */,
    const VW::multi_ex& ec_seq, VW::io::logger& /* unused */)
{
  const auto& head_ec = *ec_seq[0];
  const VW::v_array<VW::action_score>& preds = head_ec.pred.a_s;
  const auto* predicted_ec = get_example_with_labelled_class(preds[0].action, ec_seq);
  VW::details::print_cs_update_action_scores(all, predicted_ec == nullptr, cs_count_features(ec_seq), preds);
}

void update_stats_csoaa_ldf_prob(const VW::workspace& all, VW::shared_data& sd, const ldf& /* data */,
    const VW::multi_ex& ec_seq, VW::io::logger& logger)
{
  if (test_ldf_sequence(ec_seq, logger)) { sd.weighted_unlabeled_examples += ec_seq[0]->weight; }
  else { sd.weighted_labeled_examples += ec_seq[0]->weight; }
  sd.example_number++;

  size_t num_features = cs_count_features(ec_seq);
  sd.total_features += num_features;

  uint32_t predicted_class = 0;
  // Predicted class is the index of the example with the highest probability.
  const auto& probs = ec_seq[0]->pred.scalars;
  const auto max_prob_it = std::max_element(probs.begin(), probs.end());
  auto max_prob_index = std::distance(probs.begin(), max_prob_it);
  predicted_class = ec_seq[max_prob_index]->l.cs.costs[0].class_index;

  // standard loss calc
  const auto* predicted_ec = get_example_with_labelled_class(predicted_class, ec_seq);
  if (predicted_ec != nullptr)
  {
    // The loss of this example is the cost of the predicted class.
    const auto loss = predicted_ec->l.cs.costs[0].x;
    // Is this assert correct?
    assert(loss >= 0);
    sd.sum_loss += loss;
    sd.sum_loss_since_last_dump += loss;
  }

  // Multiclass loss calc
  float min_cost = FLT_MAX;
  size_t correct_class_k = 0;

  for (size_t k = 0; k < ec_seq.size(); k++)
  {
    float ec_cost = ec_seq[k]->l.cs.costs[0].x;
    if (ec_cost < min_cost)
    {
      min_cost = ec_cost;
      correct_class_k = k;
    }
  }

  float multiclass_log_loss = 999;  // -log(0) = plus infinity
  const float correct_class_prob = ec_seq[0]->pred.scalars[correct_class_k];
  if (correct_class_prob > 0.f) { multiclass_log_loss = -std::log(correct_class_prob); }

  // TODO: How to detect if we should update holdout or normal loss?
  // (ec.test_only) OR (COST_SENSITIVE::example_is_test(ec))
  // What should be the "ec"? data.ec_seq[0]?
  // Based on parse_args.cc (where "average multiclass log loss") is printed,
  // I decided to try yet another way: (!all.holdout_set_off).
  if (!all.holdout_set_off) { sd.holdout_multiclass_log_loss += multiclass_log_loss; }
  else { sd.multiclass_log_loss += multiclass_log_loss; }
}

void output_example_prediction_csoaa_ldf_prob(
    VW::workspace& all, const ldf& /* data */, const VW::multi_ex& ec_seq, VW::io::logger& logger)
{
  for (auto& sink : all.final_prediction_sink)
  {
    for (const auto prob : ec_seq[0]->pred.scalars) { all.print_by_ref(sink.get(), prob, 0, ec_seq[0]->tag, logger); }
  }
  if (all.raw_prediction != nullptr) { csoaa_ldf_print_raw(all, all.raw_prediction.get(), ec_seq, logger); }
  VW::details::global_print_newline(all.final_prediction_sink, logger);
}

void print_update_csoaa_ldf_prob(VW::workspace& all, VW::shared_data& /* sd */, const ldf& /* data */,
    const VW::multi_ex& ec_seq, VW::io::logger& /* unused */)
{
  uint32_t predicted_class = 0;
  // Predicted class is the index of the example with the highest probability.
  const auto& probs = ec_seq[0]->pred.scalars;
  const auto max_prob_it = std::max_element(probs.begin(), probs.end());
  auto max_prob_index = std::distance(probs.begin(), max_prob_it);
  predicted_class = ec_seq[max_prob_index]->l.cs.costs[0].class_index;

  // standard loss calc
  const auto* predicted_ec = get_example_with_labelled_class(predicted_class, ec_seq);

  VW::details::print_cs_update_multiclass(all, predicted_ec == nullptr, cs_count_features(ec_seq), predicted_class);
}

void update_stats_csoaa_ldf_multiclass(const VW::workspace& /* all */, VW::shared_data& sd, const ldf& /* data */,
    const VW::multi_ex& ec_seq, VW::io::logger& logger)
{
  if (test_ldf_sequence(ec_seq, logger)) { sd.weighted_unlabeled_examples += ec_seq[0]->weight; }
  else { sd.weighted_labeled_examples += ec_seq[0]->weight; }
  sd.example_number++;

  size_t num_features = cs_count_features(ec_seq);
  sd.total_features += num_features;

  const uint32_t predicted_class = ec_seq[0]->pred.multiclass;

  const auto* predicted_ec = get_example_with_labelled_class(predicted_class, ec_seq);
  if (predicted_ec != nullptr)
  {
    // The loss of this example is the cost of the predicted class.
    const auto loss = predicted_ec->l.cs.costs[0].x;
    // Is this assert correct?
    assert(loss >= 0);
    sd.sum_loss += loss;
    sd.sum_loss_since_last_dump += loss;
  }
}

void output_example_prediction_csoaa_ldf_multiclass(
    VW::workspace& all, const ldf& /* data */, const VW::multi_ex& ec_seq, VW::io::logger& logger)
{
  for (auto& sink : all.final_prediction_sink) { csoaa_ldf_multiclass_printline(all, sink.get(), ec_seq, logger); }
  if (all.raw_prediction != nullptr) { csoaa_ldf_print_raw(all, all.raw_prediction.get(), ec_seq, logger); }
  VW::details::global_print_newline(all.final_prediction_sink, logger);
}

void print_update_csoaa_ldf_multiclass(VW::workspace& all, VW::shared_data& /* sd */, const ldf& /* data */,
    const VW::multi_ex& ec_seq, VW::io::logger& /* unused */)
{
  const uint32_t predicted_class = ec_seq[0]->pred.multiclass;
  const auto* predicted_ec = get_example_with_labelled_class(predicted_class, ec_seq);
  VW::details::print_cs_update_multiclass(all, predicted_ec == nullptr, cs_count_features(ec_seq), predicted_class);
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::csldf_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto ld = VW::make_unique<ldf>();

  std::string csoaa_ldf;
  std::string ldf_override;
  std::string wap_ldf;

  option_group_definition csldf_outer_options(
      "[Reduction] Cost Sensitive One Against All with Label Dependent Features");
  csldf_outer_options
      .add(make_option("csoaa_ldf", csoaa_ldf)
               .keep()
               .necessary()
               .one_of({"m", "multiline", "mc", "multiline-classifier"})
               .help("Use one-against-all multiclass learning with label dependent features"))
      .add(make_option("ldf_override", ldf_override)
               .help("Override singleline or multiline from csoaa_ldf or wap_ldf, eg if stored in file"))
      .add(make_option("csoaa_rank", ld->rank).keep().help("Return actions sorted by score order"))
      .add(make_option("probabilities", ld->is_probabilities).keep().help("Predict probabilities of all classes"));

  option_group_definition csldf_inner_options(
      "[Reduction] Cost Sensitive Weighted All-Pairs with Label Dependent Features");
  csldf_inner_options.add(make_option("wap_ldf", wap_ldf)
                              .keep()
                              .necessary()
                              .one_of({"m", "multiline", "mc", "multiline-classifier"})
                              .help("Use weighted all-pairs multiclass learning with label dependent features. "
                                    "Specify singleline or multiline."));

  if (!options.add_parse_and_check_necessary(csldf_outer_options))
  {
    if (!options.add_parse_and_check_necessary(csldf_inner_options)) { return nullptr; }
  }

  // csoaa_ldf does logistic link manually for probabilities because the unlinked values are
  // required elsewhere. This implemenation will provide correct probabilities regardless
  // of whether --link logistic is included or not.
  if (ld->is_probabilities && options.was_supplied("link")) { options.replace("link", "identity"); }

  ld->all = &all;

  std::string ldf_arg;

  if (options.was_supplied("csoaa_ldf")) { ldf_arg = csoaa_ldf; }
  else
  {
    ldf_arg = wap_ldf;
    ld->is_wap = true;
  }
  if (options.was_supplied("ldf_override")) { ldf_arg = ldf_override; }

  ld->treat_as_classifier = false;
  if (ldf_arg == "multiline" || ldf_arg == "m") { ld->treat_as_classifier = false; }
  else if (ldf_arg == "multiline-classifier" || ldf_arg == "mc") { ld->treat_as_classifier = true; }
  else
  {
    if (all.training) THROW("ldf requires either m/multiline or mc/multiline-classifier");
    if ((ldf_arg == "singleline" || ldf_arg == "s") || (ldf_arg == "singleline-classifier" || ldf_arg == "sc"))
      THROW(
          "ldf requires either m/multiline or mc/multiline-classifier.  s/sc/singleline/singleline-classifier is no "
          "longer supported");
  }

  if (ld->is_probabilities)
  {
    all.sd->report_multiclass_log_loss = true;
    auto loss_function_type = all.loss->get_type();
    if (loss_function_type != "logistic")
    {
      all.logger.out_warn(
          "--probabilities should be used only with --loss_function=logistic, currently using: {}", loss_function_type);
    }
    if (!ld->treat_as_classifier)
    {
      all.logger.out_warn("--probabilities should be used with --csoaa_ldf=mc (or --oaa, --multilabel_oaa)");
    }
  }

  all.example_parser->emptylines_separate_examples = true;  // TODO: check this to be sure!!!  !ld->is_singleline;

  ld->label_features.max_load_factor(0.25);
  ld->label_features.reserve(256);

  auto base = require_singleline(stack_builder.setup_base_learner());
  VW::learner_update_stats_func<ldf, VW::multi_ex>* update_stats_func = nullptr;
  VW::learner_output_example_prediction_func<ldf, VW::multi_ex>* output_example_prediction_func = nullptr;
  VW::learner_print_update_func<ldf, VW::multi_ex>* print_update_func = nullptr;

  std::string name = stack_builder.get_setupfn_name(csldf_setup);
  std::string name_addition;
  VW::prediction_type_t pred_type;
  void (*pred_ptr)(ldf&, learner&, VW::multi_ex&);
  if (ld->rank)
  {
    name_addition = "-rank";
    pred_type = VW::prediction_type_t::ACTION_SCORES;
    pred_ptr = predict_csoaa_ldf_rank;
    update_stats_func = update_stats_csoaa_ldf_rank;
    output_example_prediction_func = output_example_prediction_csoaa_ldf_rank;
    print_update_func = print_update_csoaa_ldf_rank;
  }
  else if (ld->is_probabilities)
  {
    name_addition = "-prob";
    pred_type = VW::prediction_type_t::SCALARS;
    pred_ptr = predict_csoaa_ldf_probabilities;
    update_stats_func = update_stats_csoaa_ldf_prob;
    output_example_prediction_func = output_example_prediction_csoaa_ldf_prob;
    print_update_func = print_update_csoaa_ldf_prob;
  }
  else
  {
    name_addition = "";
    pred_type = VW::prediction_type_t::MULTICLASS;
    pred_ptr = predict_csoaa_ldf;
    update_stats_func = update_stats_csoaa_ldf_multiclass;
    output_example_prediction_func = output_example_prediction_csoaa_ldf_multiclass;
    print_update_func = print_update_csoaa_ldf_multiclass;
  }

  auto l = make_reduction_learner(std::move(ld), base, learn_csoaa_ldf, pred_ptr, name + name_addition)
               .set_end_pass(end_pass)
               .set_input_label_type(VW::label_type_t::CS)
               .set_output_label_type(VW::label_type_t::SIMPLE)
               .set_input_prediction_type(VW::prediction_type_t::SCALAR)
               .set_output_prediction_type(pred_type)
               .set_update_stats(update_stats_func)
               .set_output_example_prediction(output_example_prediction_func)
               .set_print_update(print_update_func)
               .build();

  all.example_parser->lbl_parser = VW::cs_label_parser_global;

  return l;
}
