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
#include "vw/core/loss_functions.h"
#include "vw/core/print_utils.h"
#include "vw/core/reductions/gd.h"  // GD::foreach_feature() needed in subtract_example()
#include "vw/core/scope_exit.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"

#include <algorithm>
#include <cerrno>
#include <cfloat>
#include <cmath>

using namespace VW::LEARNER;
using namespace COST_SENSITIVE;
using namespace VW::config;
using namespace ACTION_SCORE;

#undef VW_DEBUG_LOG
#define VW_DEBUG_LOG vw_dbg::csoaa_ldf

namespace
{
// TODO: passthrough for ldf
struct ldf
{
  LabelDict::label_feature_map label_features;

  bool is_wap = false;
  bool treat_as_classifier = false;
  bool is_probabilities = false;
  VW::workspace* all = nullptr;

  bool rank = false;
  action_scores a_s;
  uint64_t ft_offset = 0;

  std::vector<action_scores> stored_preds;
};

inline bool cmp_wclass_ptr(const COST_SENSITIVE::wclass* a, const COST_SENSITIVE::wclass* b) { return a->x < b->x; }

void compute_wap_values(std::vector<COST_SENSITIVE::wclass*> costs)
{
  std::sort(costs.begin(), costs.end(), cmp_wclass_ptr);
  costs[0]->wap_value = 0.;
  for (size_t i = 1; i < costs.size(); i++)
  { costs[i]->wap_value = costs[i - 1]->wap_value + (costs[i]->x - costs[i - 1]->x) / static_cast<float>(i); }
}

// Substract a given feature from example ec.
// Rather than finding the corresponding namespace and feature in ec,
// add a new feature with opposite value (but same index) to ec to a special wap_ldf_namespace.
// This is faster and allows fast undo in unsubtract_example().
void subtract_feature(VW::example& ec, float feature_value_x, uint64_t weight_index)
{
  ec.feature_space[wap_ldf_namespace].push_back(-feature_value_x, weight_index, wap_ldf_namespace);
}

// Iterate over all features of ecsub including quadratic and cubic features and subtract them from ec.
void subtract_example(VW::workspace& all, VW::example* ec, VW::example* ecsub)
{
  features& wap_fs = ec->feature_space[wap_ldf_namespace];
  wap_fs.sum_feat_sq = 0;
  GD::foreach_feature<VW::example&, uint64_t, subtract_feature>(all, *ecsub, *ec);
  ec->indices.push_back(wap_ldf_namespace);
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

  if (ec->indices.back() != wap_ldf_namespace)
  {
    logger.err_error(
        "Internal error (bug): trying to unsubtract_example, but either it wasn't added, or something was added "
        "after and not removed");
    return;
  }

  features& fs = ec->feature_space[wap_ldf_namespace];
  ec->num_features -= fs.size();
  ec->reset_total_sum_feat_sq();
  fs.clear();
  ec->indices.pop_back();
}

void make_single_prediction(ldf& data, single_learner& base, VW::example& ec)
{
  uint64_t old_offset = ec.ft_offset;

  LabelDict::add_example_namespace_from_memory(data.label_features, ec, ec.l.cs.costs[0].class_index);

  auto restore_guard = VW::scope_exit([&data, old_offset, &ec] {
    ec.ft_offset = old_offset;
    // WARNING: Access of label information when making prediction is
    // problematic.
    ec.l.cs.costs[0].partial_prediction = ec.partial_prediction;
    // WARNING: Access of label information when making prediction is
    // problematic.
    LabelDict::del_example_namespace_from_memory(data.label_features, ec, ec.l.cs.costs[0].class_index);
  });

  ec.l.simple = label_data{FLT_MAX};
  ec._reduction_features.template get<simple_label_reduction_features>().reset_to_default();

  ec.ft_offset = data.ft_offset;
  base.predict(ec);  // make a prediction
}

bool test_ldf_sequence(ldf& /*data*/, const VW::multi_ex& ec_seq, VW::io::logger& logger)
{
  bool isTest;
  if (ec_seq.empty()) { isTest = true; }
  else
  {
    isTest = COST_SENSITIVE::cs_label.test_label(ec_seq[0]->l);
  }
  for (const auto& ec : ec_seq)
  {
    // Each sub-example must have just one cost
    assert(ec->l.cs.costs.size() == 1);

    if (COST_SENSITIVE::cs_label.test_label(ec->l) != isTest)
    {
      isTest = true;
      logger.err_warn("ldf example has mix of train/test data; assuming test");
    }
  }
  return isTest;
}

void do_actual_learning_wap(ldf& data, single_learner& base, VW::multi_ex& ec_seq)
{
  VW_DBG(ec_seq) << "do_actual_learning_wap()" << std::endl;

  size_t K = ec_seq.size();
  std::vector<COST_SENSITIVE::wclass*> all_costs;
  for (const auto& example : ec_seq) { all_costs.push_back(&example->l.cs.costs[0]); }
  compute_wap_values(all_costs);

  for (size_t k1 = 0; k1 < K; k1++)
  {
    VW::example* ec1 = ec_seq[k1];

    // save original variables
    COST_SENSITIVE::label save_cs_label = ec1->l.cs;
    label_data& simple_lbl = ec1->l.simple;
    auto& simple_red_features = ec1->_reduction_features.template get<simple_label_reduction_features>();

    auto costs1 = save_cs_label.costs;
    if (costs1[0].class_index == static_cast<uint32_t>(-1)) { continue; }

    LabelDict::add_example_namespace_from_memory(data.label_features, *ec1, costs1[0].class_index);

    // Guard example state restore against throws
    auto restore_guard = VW::scope_exit([&data, &save_cs_label, &costs1, &ec1] {
      LabelDict::del_example_namespace_from_memory(data.label_features, *ec1, costs1[0].class_index);

      // restore original cost-sensitive label, sum of importance weights
      ec1->l.cs = save_cs_label;
    });

    for (size_t k2 = k1 + 1; k2 < K; k2++)
    {
      VW::example* ec2 = ec_seq[k2];
      auto costs2 = ec2->l.cs.costs;

      if (costs2[0].class_index == static_cast<uint32_t>(-1)) { continue; }
      float value_diff = std::fabs(costs2[0].wap_value - costs1[0].wap_value);
      // float value_diff = fabs(costs2[0].x - costs1[0].x);
      if (value_diff < 1e-6) { continue; }

      // Prepare example for learning
      LabelDict::add_example_namespace_from_memory(data.label_features, *ec2, costs2[0].class_index);

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
      auto restore_guard_inner = VW::scope_exit([&data, old_offset, old_weight, &costs2, &ec2, &ec1] {
        ec1->ft_offset = old_offset;
        ec1->weight = old_weight;
        unsubtract_example(ec1, data.all->logger);
        LabelDict::del_example_namespace_from_memory(data.label_features, *ec2, costs2[0].class_index);
      });

      base.learn(*ec1);
    }
    // TODO: What about partial_prediction? See do_actual_learning_oaa.
  }
}

void do_actual_learning_oaa(ldf& data, single_learner& base, VW::multi_ex& ec_seq)
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
    label save_cs_label = std::move(ec->l.cs);
    const auto& costs = save_cs_label.costs;

    // build example for the base learner
    label_data simple_lbl;

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
    auto& simple_red_features = ec->_reduction_features.template get<simple_label_reduction_features>();
    simple_red_features.initial = 0.;
    ec->l.simple = simple_lbl;

    // Prepare examples for learning
    LabelDict::add_example_namespace_from_memory(data.label_features, *ec, costs[0].class_index);
    uint64_t old_offset = ec->ft_offset;
    ec->ft_offset = data.ft_offset;

    // Guard example state restore against throws
    auto restore_guard = VW::scope_exit([&save_cs_label, &data, &costs, old_offset, old_weight, &ec] {
      ec->ft_offset = old_offset;
      LabelDict::del_example_namespace_from_memory(data.label_features, *ec, costs[0].class_index);
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
void learn_csoaa_ldf(ldf& data, single_learner& base, VW::multi_ex& ec_seq_all)
{
  if (ec_seq_all.empty())
  {
    return;  // nothing to do
  }

  data.ft_offset = ec_seq_all[0]->ft_offset;

  /////////////////////// learn
  if (!test_ldf_sequence(data, ec_seq_all, data.all->logger))
  {
    if (data.is_wap) { do_actual_learning_wap(data, base, ec_seq_all); }
    else
    {
      do_actual_learning_oaa(data, base, ec_seq_all);
    }
  }
}

void convert_to_probabilities(VW::multi_ex& ec_seq)
{
  float sum_prob = 0;
  for (const auto& example : ec_seq)
  {
    // probability(correct_class) = 1 / (1+exp(-score)), where score is higher
    // for better classes,
    // but partial_prediction is lower for better classes (we are predicting the
    // cost),
    // so we need to take score = -partial_prediction,
    // thus probability(correct_class) = 1 / (1+exp(-(-partial_prediction)))
    float prob = 1.f / (1.f + correctedExp(example->partial_prediction));
    example->pred.prob = prob;
    sum_prob += prob;
  }
  // make sure that the probabilities sum up (exactly) to one
  for (const auto& example : ec_seq) { example->pred.prob /= sum_prob; }
}

/*
 * 1) process all labels at first
 * 2) verify no labels in the middle of data
 * 3) learn_or_predict(data) with rest
 */
void predict_csoaa_ldf(ldf& data, single_learner& base, VW::multi_ex& ec_seq_all)
{
  if (ec_seq_all.empty())
  {
    return;  // nothing to do
  }

  data.ft_offset = ec_seq_all[0]->ft_offset;

  uint32_t K = static_cast<uint32_t>(ec_seq_all.size());
  uint32_t predicted_K = 0;

  auto restore_guard = VW::scope_exit([&data, &ec_seq_all, K, &predicted_K] {
    // Mark the predicted sub-example with its class_index, all other with 0
    for (size_t k = 0; k < K; k++)
    {
      if (k == predicted_K) { ec_seq_all[k]->pred.multiclass = ec_seq_all[k]->l.cs.costs[0].class_index; }
      else
      {
        ec_seq_all[k]->pred.multiclass = 0;
      }
    }

    ////////////////////// compute probabilities
    if (data.is_probabilities) { convert_to_probabilities(ec_seq_all); }
  });

  /////////////////////// do prediction
  float min_score = FLT_MAX;
  for (uint32_t k = 0; k < K; k++)
  {
    VW::example* ec = ec_seq_all[k];
    make_single_prediction(data, base, *ec);
    if (ec->partial_prediction < min_score)
    {
      min_score = ec->partial_prediction;
      predicted_K = k;
    }
  }
}

/*
 * 1) process all labels at first
 * 2) verify no labels in the middle of data
 * 3) learn_or_predict(data) with rest
 */
void predict_csoaa_ldf_rank(ldf& data, single_learner& base, VW::multi_ex& ec_seq_all)
{
  data.ft_offset = ec_seq_all[0]->ft_offset;
  if (ec_seq_all.empty())
  {
    return;  // nothing more to do
  }

  uint32_t K = static_cast<uint32_t>(ec_seq_all.size());

  /////////////////////// do prediction
  data.a_s.clear();
  data.stored_preds.clear();

  auto restore_guard = VW::scope_exit([&data, &ec_seq_all, K] {
    std::sort(data.a_s.begin(), data.a_s.end(), VW::action_score_compare_lt);

    data.stored_preds[0].clear();
    for (size_t k = 0; k < K; k++)
    {
      ec_seq_all[k]->pred.a_s = std::move(data.stored_preds[k]);
      ec_seq_all[0]->pred.a_s.push_back(data.a_s[k]);
    }

    ////////////////////// compute probabilities
    if (data.is_probabilities) { convert_to_probabilities(ec_seq_all); }
  });

  for (uint32_t k = 0; k < K; k++)
  {
    VW::example* ec = ec_seq_all[k];
    data.stored_preds.emplace_back(std::move(ec->pred.a_s));
    make_single_prediction(data, base, *ec);
    action_score s;
    s.score = ec->partial_prediction;
    s.action = ec->l.cs.costs[0].class_index;
    data.a_s.push_back(s);
  }
}

void output_example(
    VW::workspace& all, const VW::example& ec, bool& hit_loss, const VW::multi_ex* ec_seq, const ldf& data)
{
  const label& ld = ec.l.cs;
  const auto& costs = ld.costs;

  if (VW::example_is_newline(ec)) { return; }

  if (COST_SENSITIVE::ec_is_example_header(ec))
  {
    all.sd->total_features +=
        (ec_seq->size() - 1) * (ec.get_num_features() - ec.feature_space[constant_namespace].size());
    return;
  }
  else
  {
    all.sd->total_features += ec.get_num_features();
  }

  float loss = 0.f;

  uint32_t predicted_class = 0;
  if (data.is_probabilities)
  {
    // predicted_K was already computed in do_actual_learning(),
    // but we cannot store it in ec.pred union because we store ec.pred.prob there.
    // So we must compute it again.
    uint32_t predicted_K = 0;
    float min_score = FLT_MAX;
    for (size_t k = 0; k < ec_seq->size(); k++)
    {
      VW::example* ec_k = (*ec_seq)[k];
      if (ec_k->partial_prediction < min_score)
      {
        min_score = ec_k->partial_prediction;
        predicted_K = static_cast<uint32_t>(k);
      }
    }
    predicted_class = (*ec_seq)[predicted_K]->l.cs.costs[0].class_index;
  }
  else
  {
    predicted_class = ec.pred.multiclass;
  }

  if (!COST_SENSITIVE::cs_label.test_label(ec.l))
  {
    for (auto const& cost : costs)
    {
      if (hit_loss) { break; }
      if (predicted_class == cost.class_index)
      {
        loss = cost.x;
        hit_loss = true;
      }
    }

    all.sd->sum_loss += loss;
    all.sd->sum_loss_since_last_dump += loss;
  }

  for (auto& sink : all.final_prediction_sink)
  {
    all.print_by_ref(sink.get(), data.is_probabilities ? ec.pred.prob : static_cast<float>(ec.pred.multiclass), 0,
        ec.tag, all.logger);
  }

  if (all.raw_prediction != nullptr)
  {
    std::string outputString;
    std::stringstream outputStringStream(outputString);
    for (size_t i = 0; i < costs.size(); i++)
    {
      if (i > 0) { outputStringStream << ' '; }
      outputStringStream << costs[i].class_index << ':' << costs[i].partial_prediction;
    }
    // outputStringStream << std::endl;
    all.print_text_by_ref(all.raw_prediction.get(), outputStringStream.str(), ec.tag, all.logger);
  }

  COST_SENSITIVE::print_update(all, COST_SENSITIVE::cs_label.test_label(ec.l), ec, ec_seq, false, predicted_class);
}

void output_rank_example(VW::workspace& all, VW::example& head_ec, bool& hit_loss, VW::multi_ex* ec_seq)
{
  const auto& costs = head_ec.l.cs.costs;

  if (VW::example_is_newline(head_ec)) { return; }
  if (COST_SENSITIVE::ec_is_example_header(head_ec)) { return; }

  all.sd->total_features += head_ec.get_num_features();

  float loss = 0.;
  VW::v_array<action_score>& preds = head_ec.pred.a_s;

  if (!COST_SENSITIVE::cs_label.test_label(head_ec.l))
  {
    for (VW::example* ex : *ec_seq)
    {
      if (hit_loss) { break; }
      if (preds[0].action == ex->l.cs.costs[0].class_index)
      {
        loss = ex->l.cs.costs[0].x;
        hit_loss = true;
      }
    }
    all.sd->sum_loss += loss;
    all.sd->sum_loss_since_last_dump += loss;
    assert(loss >= 0);
  }

  for (auto& sink : all.final_prediction_sink)
  { print_action_score(sink.get(), head_ec.pred.a_s, head_ec.tag, all.logger); }

  if (all.raw_prediction != nullptr)
  {
    std::string outputString;
    std::stringstream outputStringStream(outputString);
    for (size_t i = 0; i < costs.size(); i++)
    {
      if (i > 0) { outputStringStream << ' '; }
      outputStringStream << costs[i].class_index << ':' << costs[i].partial_prediction;
    }
    // outputStringStream << std::endl;
    all.print_text_by_ref(all.raw_prediction.get(), outputStringStream.str(), head_ec.tag, all.logger);
  }

  COST_SENSITIVE::print_update(all, COST_SENSITIVE::cs_label.test_label(head_ec.l), head_ec, ec_seq, true, 0);
}

void output_example_seq(VW::workspace& all, ldf& data, VW::multi_ex& ec_seq)
{
  size_t K = ec_seq.size();
  if (K > 0)
  {
    if (test_ldf_sequence(data, ec_seq, all.logger)) { all.sd->weighted_unlabeled_examples += ec_seq[0]->weight; }
    else
    {
      all.sd->weighted_labeled_examples += ec_seq[0]->weight;
    }
    all.sd->example_number++;

    bool hit_loss = false;
    if (data.rank) { output_rank_example(all, **(ec_seq.begin()), hit_loss, &(ec_seq)); }
    else
    {
      for (VW::example* ec : ec_seq) { output_example(all, *ec, hit_loss, &(ec_seq), data); }
    }

    if (all.raw_prediction != nullptr)
    {
      const VW::v_array<char> empty;
      all.print_text_by_ref(all.raw_prediction.get(), "", empty, all.logger);
    }

    if (data.is_probabilities)
    {
      float min_cost = FLT_MAX;
      size_t correct_class_k = 0;

      for (size_t k = 0; k < K; k++)
      {
        float ec_cost = ec_seq[k]->l.cs.costs[0].x;
        if (ec_cost < min_cost)
        {
          min_cost = ec_cost;
          correct_class_k = k;
        }
      }

      float multiclass_log_loss = 999;  // -log(0) = plus infinity
      float correct_class_prob = ec_seq[correct_class_k]->pred.prob;
      if (correct_class_prob > 0) { multiclass_log_loss = -std::log(correct_class_prob); }

      // TODO: How to detect if we should update holdout or normal loss?
      // (ec.test_only) OR (COST_SENSITIVE::example_is_test(ec))
      // What should be the "ec"? data.ec_seq[0]?
      // Based on parse_args.cc (where "average multiclass log loss") is printed,
      // I decided to try yet another way: (!all.holdout_set_off).
      if (!all.holdout_set_off) { all.sd->holdout_multiclass_log_loss += multiclass_log_loss; }
      else
      {
        all.sd->multiclass_log_loss += multiclass_log_loss;
      }
    }
  }
}

void end_pass(ldf&) {}

void finish_multiline_example(VW::workspace& all, ldf& data, VW::multi_ex& ec_seq)
{
  if (!ec_seq.empty())
  {
    if (ec_seq.size() > 1 && COST_SENSITIVE::ec_is_example_header(*ec_seq[0]))
    {
      std::swap(ec_seq[0]->pred, ec_seq[1]->pred);
      std::swap(ec_seq[0]->tag, ec_seq[1]->tag);
    }
    output_example_seq(all, data, ec_seq);
    if (ec_seq.size() > 1 && COST_SENSITIVE::ec_is_example_header(*ec_seq[0]))
    {
      std::swap(ec_seq[0]->pred, ec_seq[1]->pred);
      std::swap(ec_seq[0]->tag, ec_seq[1]->tag);
    }
    VW::details::global_print_newline(all.final_prediction_sink, all.logger);
  }

  VW::finish_example(all, ec_seq);
}
}  // namespace

base_learner* VW::reductions::csldf_setup(VW::setup_base_i& stack_builder)
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
  else if (ldf_arg == "multiline-classifier" || ldf_arg == "mc")
  {
    ld->treat_as_classifier = true;
  }
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
    { all.logger.out_warn("--probabilities should be used with --csoaa_ldf=mc (or --oaa, --multilabel_oaa)"); }
  }

  all.example_parser->emptylines_separate_examples = true;  // TODO: check this to be sure!!!  !ld->is_singleline;

  ld->label_features.max_load_factor(0.25);
  ld->label_features.reserve(256);

  single_learner* pbase = as_singleline(stack_builder.setup_base_learner());

  std::string name = stack_builder.get_setupfn_name(csldf_setup);
  std::string name_addition;
  VW::prediction_type_t pred_type;
  void (*pred_ptr)(ldf&, single_learner&, VW::multi_ex&);
  if (ld->rank)
  {
    name_addition = "-rank";
    pred_type = VW::prediction_type_t::action_scores;
    pred_ptr = predict_csoaa_ldf_rank;
  }
  else if (ld->is_probabilities)
  {
    name_addition = "-prob";
    pred_type = VW::prediction_type_t::prob;
    pred_ptr = predict_csoaa_ldf;
  }
  else
  {
    name_addition = "";
    pred_type = VW::prediction_type_t::multiclass;
    pred_ptr = predict_csoaa_ldf;
  }

  auto* l = make_reduction_learner(std::move(ld), pbase, learn_csoaa_ldf, pred_ptr, name + name_addition)
                .set_finish_example(finish_multiline_example)
                .set_end_pass(end_pass)
                .set_input_label_type(VW::label_type_t::cs)
                .set_output_prediction_type(pred_type)
                .build();

  all.example_parser->lbl_parser = COST_SENSITIVE::cs_label;
  all.cost_sensitive = make_base(*l);
  return all.cost_sensitive;
}
