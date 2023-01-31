// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "vw/core/reductions/oaa.h"

#include "vw/common/random.h"
#include "vw/common/vw_exception.h"
#include "vw/config/options.h"
#include "vw/core/correctedMath.h"
#include "vw/core/learner.h"
#include "vw/core/loss_functions.h"
#include "vw/core/multiclass.h"
#include "vw/core/named_labels.h"
#include "vw/core/prediction_type.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"

#include <cfloat>
#include <cmath>
#include <cstddef>
#include <sstream>

using namespace VW::config;

namespace
{
static constexpr bool PRINT_ALL = true;
static constexpr bool SCORES = true;
static constexpr bool PROBABILITIES = true;

class oaa
{
public:
  uint64_t k = 0;
  VW::workspace* all = nullptr;         // for raw
  VW::polyprediction* pred = nullptr;   // for multipredict
  uint64_t num_subsample = 0;           // for randomized subsampling, how many negatives to draw?
  uint32_t* subsample_order = nullptr;  // for randomized subsampling, in what order should we touch classes
  size_t subsample_id = 0;              // for randomized subsampling, where do we live in the list
  VW::io::logger logger;
  // Default value of 2 follows behavior of 1-indexing and can change to 0-indexing if detected
  uint32_t& indexing;  // for 0 or 1 indexing

  oaa(VW::io::logger logger, uint32_t& indexing) : logger(std::move(logger)), indexing(indexing) {}

  ~oaa()
  {
    free(pred);
    free(subsample_order);
  }
};

void learn_randomized(oaa& o, VW::LEARNER::learner& base, VW::example& ec)
{
  // Update indexing
  if (o.indexing == 2 && ec.l.multi.label == 0)
  {
    o.logger.out_info("label 0 found -- labels are now considered 0-indexed.");
    o.indexing = 0;
  }
  else if (o.indexing == 2 && ec.l.multi.label == o.k)
  {
    o.logger.out_info("label {0} found -- labels are now considered 1-indexed.", o.k);
    o.indexing = 1;
  }

  VW::multiclass_label ld = ec.l.multi;

  // Label validation
  if (o.indexing == 0 && ld.label >= o.k)
  {
    o.all->logger.out_warn("label {0} is not in {{0,{1}}}. This won't work for 0-indexed actions.", ld.label, o.k - 1);
    ec.l.multi.label = 0;
  }
  else if (o.indexing == 1 && (ld.label < 1 || ld.label > o.k))
  {
    o.all->logger.out_warn("label {0} is not in {{1,{1}}}. This won't work for 1-indexed actions.", ld.label, o.k);
    ec.l.multi.label = static_cast<uint32_t>(o.k);
  }

  ec.l.simple.label = 1.;  // truth
  ec.ex_reduction_features.template get<VW::simple_label_reduction_features>().reset_to_default();
  uint32_t lbl_ind = (o.indexing == 0) ? ld.label : ld.label - 1;

  base.learn(ec, lbl_ind);

  size_t prediction = ld.label;
  float best_partial_prediction = ec.partial_prediction;

  ec.l.simple.label = -1.;
  float weight_temp = ec.weight;
  ec.weight *= (static_cast<float>(o.k)) / static_cast<float>(o.num_subsample);
  size_t p = o.subsample_id;
  size_t count = 0;
  while (count < o.num_subsample)
  {
    uint32_t l = o.subsample_order[p];
    p = (p + 1) % o.k;
    if (l == (ld.label + o.k - 1) % o.k) { continue; }
    base.learn(ec, l);
    if (ec.partial_prediction > best_partial_prediction)
    {
      best_partial_prediction = ec.partial_prediction;
      prediction = l + 1;
      if (o.indexing == 0 && prediction == o.k) { prediction = 0; }
    }
    count++;
  }
  o.subsample_id = p;

  ec.pred.multiclass = static_cast<uint32_t>(prediction);
  ec.l.multi = ld;
  ec.weight = weight_temp;
}

template <bool print_all, bool scores, bool probabilities>
void learn(oaa& o, VW::LEARNER::learner& base, VW::example& ec)
{
  // Update indexing
  if (o.indexing == 2 && ec.l.multi.label == 0)
  {
    o.logger.out_info("label 0 found -- labels are now considered 0-indexed.");
    o.indexing = 0;
  }
  else if (o.indexing == 2 && ec.l.multi.label == o.k)
  {
    o.logger.out_info("label {0} found -- labels are now considered 1-indexed.", o.k);
    o.indexing = 1;
  }

  // Save label
  VW::multiclass_label mc_label_data = ec.l.multi;

  // Label validation
  if (o.indexing == 0 && mc_label_data.label >= o.k)
  {
    o.all->logger.out_warn(
        "label {0} is not in {{0,{1}}}. This won't work for 0-indexed actions.", mc_label_data.label, o.k - 1);
    ec.l.multi.label = 0;
  }
  else if (o.indexing == 1 && (mc_label_data.label < 1 || mc_label_data.label > o.k))
  {
    o.all->logger.out_warn(
        "label {0} is not in {{1,{1}}}. This won't work for 1-indexed actions.", mc_label_data.label, o.k);
    ec.l.multi.label = static_cast<uint32_t>(o.k);
  }

  ec.l.simple = {FLT_MAX};
  ec.ex_reduction_features.template get<VW::simple_label_reduction_features>().reset_to_default();

  for (uint32_t i = 0; i < o.k; i++)
  {
    uint32_t lbl = (o.indexing == 0) ? i : i + 1;
    ec.l.simple.label = (mc_label_data.label == lbl) ? 1.f : -1.f;
    // The following is an unfortunate loss of abstraction
    // Downstream reduction (gd.update) uses the prediction
    // from here
    ec.pred.scalar = o.pred[i].scalar;
    base.update(ec, i);
  }

  // Restore label
  ec.l.multi = mc_label_data;
}

template <bool print_all, bool scores, bool probabilities>
void predict(oaa& o, VW::LEARNER::learner& base, VW::example& ec)
{
  // The predictions are either an array of scores or a single
  // class id of a multiclass label

  // In the case we return scores, we need to save a copy of
  // the pre-allocated scores array since ec.pred will be
  // used for other predictions.
  VW::v_array<float> scores_array;
  if (scores) { scores_array = ec.pred.scalars; }

  // oaa.pred - Predictions will get stored in this array
  // oaa.k    - Number of learners to call predict() on
  base.multipredict(ec, 0, o.k, o.pred, true);

  // Find the class with the largest score (index +1)
  uint32_t prediction = 0;
  for (uint32_t i = 1; i < o.k; i++)
  {
    if (o.pred[i].scalar > o.pred[prediction].scalar) { prediction = i; }
  }
  if (o.indexing != 0) { ++prediction; }

  if (ec.passthrough)
  {
    if (o.indexing == 0)
    {
      VW_ADD_PASSTHROUGH_FEATURE(ec, 0, o.pred[o.k - 1].scalar);
      for (uint32_t i = 0; i < o.k - 1; i++) { VW_ADD_PASSTHROUGH_FEATURE(ec, (i + 1), o.pred[i].scalar); }
    }
    else
    {
      for (uint32_t i = 1; i <= o.k; i++) { VW_ADD_PASSTHROUGH_FEATURE(ec, i, o.pred[i - 1].scalar); }
    }
  }

  // Print predictions to a file
  if (print_all)
  {
    std::stringstream output_string_stream;
    if (o.indexing == 0)
    {
      output_string_stream << ' ' << 0 << ':' << o.pred[o.k - 1].scalar;
      for (uint32_t i = 0; i < o.k - 1; i++) { output_string_stream << ' ' << (i + 1) << ':' << o.pred[i].scalar; }
    }
    else
    {
      for (uint32_t i = 1; i <= o.k; i++) { output_string_stream << ' ' << i << ':' << o.pred[i - 1].scalar; }
    }
    o.all->print_text_by_ref(o.all->raw_prediction.get(), output_string_stream.str(), ec.tag, o.all->logger);
  }

  // The predictions are an array of scores (as opposed to a single index of a
  // class)
  if (scores)
  {
    scores_array.clear();
    for (uint32_t i = 0; i < o.k; i++) { scores_array.push_back(o.pred[i].scalar); }
    ec.pred.scalars = scores_array;

    // The scores should be converted to probabilities
    if (probabilities)
    {
      float sum_prob = 0;
      for (uint32_t i = 0; i < o.k; i++)
      {
        ec.pred.scalars[i] = 1.f / (1.f + VW::details::correctedExp(-o.pred[i].scalar));
        sum_prob += ec.pred.scalars[i];
      }
      const float inv_sum_prob = 1.f / sum_prob;
      for (uint32_t i = 0; i < o.k; i++) { ec.pred.scalars[i] *= inv_sum_prob; }
    }
  }
  else { ec.pred.multiclass = prediction; }
}

template <bool probabilities>
void update_stats_oaa(const VW::workspace& /* all */, VW::shared_data& sd, const oaa& data, const VW::example& ec,
    VW::io::logger& /* logger */)
{
  // === Compute multiclass_log_loss
  // TODO:
  // What to do if the correct label is unknown, i.e. (uint32_t)-1?
  //   Suggestion: increase all.sd->weighted_unlabeled_examples???,
  //               but not sd.example_number, so the average loss is not influenced.
  // What to do if the correct_class_prob==0?
  //   Suggestion: have some maximal multiclass_log_loss limit, e.g. 999.
  float multiclass_log_loss = 999;  // -log(0) = plus infinity
  float correct_class_prob = 0;
  if (probabilities)
  {
    correct_class_prob = ec.pred.scalars[((data.indexing == 0) ? ec.l.multi.label : ec.l.multi.label - 1) % data.k];
    if (correct_class_prob > 0) { multiclass_log_loss = -std::log(correct_class_prob) * ec.weight; }
    if (ec.test_only) { sd.holdout_multiclass_log_loss += multiclass_log_loss; }
    else { sd.multiclass_log_loss += multiclass_log_loss; }
  }
  // === Compute `prediction` and zero_one_loss
  // We have already computed `prediction` in predict_or_learn,
  // but we cannot store it in ec.pred union because we store ec.pred.probs there.
  uint32_t prediction_ind = 0;
  for (uint32_t i = 1; i < data.k; i++)
  {
    if (data.pred[i].scalar > data.pred[prediction_ind].scalar) { prediction_ind = i; }
  }
  uint32_t pred_lbl = (data.indexing == 0) ? prediction_ind : prediction_ind + 1;

  float zero_one_loss = 0;
  if (ec.l.multi.label != pred_lbl) { zero_one_loss = ec.weight; }

  // === Report updates using zero-one loss
  sd.update(
      ec.test_only, ec.l.multi.label != static_cast<uint32_t>(-1), zero_one_loss, ec.weight, ec.get_num_features());
  // Alternatively, we could report multiclass_log_loss.
  // all.sd->update(ec.test_only, multiclass_log_loss, ec.weight, ec.num_features);
  // Even better would be to report both losses, but this would mean to increase
  // the number of columns and this would not fit narrow screens.
  // So let's report (average) multiclass_log_loss only in the final resume.
}

template <bool probabilities>
void print_update_oaa(
    VW::workspace& all, VW::shared_data& /* sd */, const oaa& data, const VW::example& ec, VW::io::logger& /* unused */)
{
  // === Compute `prediction` and zero_one_loss
  // We have already computed `prediction` in predict_or_learn,
  // but we cannot store it in ec.pred union because we store ec.pred.probs there.
  uint32_t prediction_ind = 0;
  for (uint32_t i = 1; i < data.k; i++)
  {
    if (data.pred[i].scalar > data.pred[prediction_ind].scalar) { prediction_ind = i; }
  }
  uint32_t pred_lbl = (data.indexing == 0) ? prediction_ind : prediction_ind + 1;

  // === Print progress report
  if (probabilities) { VW::details::print_multiclass_update_with_probability(all, ec, pred_lbl); }
  else { VW::details::print_multiclass_update_with_score(all, ec, pred_lbl); }
}

void output_example_prediction_oaa(
    VW::workspace& all, const oaa& data, const VW::example& ec, VW::io::logger& /* unused */)
{
  // === Print probabilities for all classes
  std::ostringstream output_string_stream;
  for (uint32_t i = 0; i < data.k; i++)
  {
    uint32_t corrected_label = (data.indexing == 0) ? i : i + 1;
    if (i > 0) { output_string_stream << ' '; }
    if (all.sd->ldict) { output_string_stream << all.sd->ldict->get(corrected_label); }
    else { output_string_stream << corrected_label; }
    output_string_stream << ':' << ec.pred.scalars[i];
  }
  const auto ss_str = output_string_stream.str();
  for (auto& sink : all.final_prediction_sink) { all.print_text_by_ref(sink.get(), ss_str, ec.tag, all.logger); }
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::oaa_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto data = VW::make_unique<oaa>(all.logger, all.indexing);
  bool probabilities = false;
  bool scores = false;
  option_group_definition new_options("[Reduction] One Against All");
  new_options.add(make_option("oaa", data->k).keep().necessary().help("One-against-all multiclass with <k> labels"))
      .add(make_option("oaa_subsample", data->num_subsample)
               .help("Subsample this number of negative examples when learning"))
      .add(make_option("probabilities", probabilities).help("Predict probabilities of all classes"))
      .add(make_option("scores", scores).help("Output raw scores per class"))
      .add(make_option("indexing", all.indexing).one_of({0, 1}).keep().help("Choose between 0 or 1-indexing"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  // oaa does logistic link manually for probabilities because the unlinked values are required
  // in base.update(). This implemenation will provide correct probabilities regardless
  // of whether --link logistic is included or not.
  if (probabilities && options.was_supplied("link")) { options.replace("link", "identity"); }

  if (all.sd->ldict && (data->k != all.sd->ldict->getK()))
    THROW("There are " << all.sd->ldict->getK() << " named labels. Use that as the argument to oaa.")

  data->all = &all;
  data->pred = VW::details::calloc_or_throw<VW::polyprediction>(data->k);
  data->subsample_order = nullptr;
  data->subsample_id = 0;
  if (data->num_subsample > 0)
  {
    if (data->num_subsample >= data->k)
    {
      data->num_subsample = 0;
      all.logger.err_info("oaa is turning off subsampling because parameter >= K");
    }
    else
    {
      data->subsample_order = VW::details::calloc_or_throw<uint32_t>(data->k);
      for (size_t i = 0; i < data->k; i++) { data->subsample_order[i] = static_cast<uint32_t>(i); }
      for (size_t i = 0; i < data->k; i++)
      {
        size_t j =
            static_cast<size_t>(all.get_random_state()->get_and_update_random() * static_cast<float>(data->k - i)) + i;
        uint32_t tmp = data->subsample_order[i];
        data->subsample_order[i] = data->subsample_order[j];
        data->subsample_order[j] = tmp;
      }
    }
  }

  oaa* data_ptr = data.get();
  uint64_t k_value = data->k;
  auto base = require_singleline(stack_builder.setup_base_learner());
  void (*learn_ptr)(oaa&, VW::LEARNER::learner&, VW::example&) = nullptr;
  void (*pred_ptr)(oaa&, VW::LEARNER::learner&, VW::example&) = nullptr;
  std::string name_addition;
  VW::prediction_type_t pred_type;

  VW::learner_update_stats_func<oaa, VW::example>* update_stats_func = nullptr;
  VW::learner_output_example_prediction_func<oaa, VW::example>* output_example_prediction_func =
      output_example_prediction_oaa;
  VW::learner_print_update_func<oaa, VW::example>* print_update_func = nullptr;
  if (probabilities || scores)
  {
    pred_type = VW::prediction_type_t::SCALARS;
    if (probabilities)
    {
      auto loss_function_type = all.loss->get_type();
      if (loss_function_type != "logistic")
      {
        all.logger.out_warn("--probabilities should be used only with --loss_function=logistic, currently using: {}",
            loss_function_type);
      }
      // the three boolean template parameters are: is_learn, print_all and scores
      learn_ptr = learn<!PRINT_ALL, SCORES, PROBABILITIES>;
      pred_ptr = predict<!PRINT_ALL, SCORES, PROBABILITIES>;
      name_addition = "-prob";
      update_stats_func = update_stats_oaa<true>;
      print_update_func = print_update_oaa<true>;
      all.sd->report_multiclass_log_loss = true;
    }
    else
    {
      learn_ptr = learn<!PRINT_ALL, SCORES, !PROBABILITIES>;
      pred_ptr = predict<!PRINT_ALL, SCORES, !PROBABILITIES>;
      name_addition = "-scores";
      update_stats_func = update_stats_oaa<false>;
      print_update_func = print_update_oaa<false>;
    }
  }
  else
  {
    pred_type = VW::prediction_type_t::MULTICLASS;
    update_stats_func = VW::details::update_stats_multiclass_label<oaa>;
    output_example_prediction_func = VW::details::output_example_prediction_multiclass_label<oaa>;
    print_update_func = VW::details::print_update_multiclass_label<oaa>;
    if (all.raw_prediction != nullptr)
    {
      learn_ptr = learn<PRINT_ALL, !SCORES, !PROBABILITIES>;
      pred_ptr = predict<PRINT_ALL, !SCORES, !PROBABILITIES>;
      name_addition = "-raw";
    }
    else
    {
      learn_ptr = learn<!PRINT_ALL, !SCORES, !PROBABILITIES>;
      pred_ptr = predict<!PRINT_ALL, !SCORES, !PROBABILITIES>;
      name_addition = "";
    }
  }

  all.example_parser->lbl_parser = VW::multiclass_label_parser_global;

  if (data_ptr->num_subsample > 0)
  {
    learn_ptr = learn_randomized;
    // Override the update stats func.
    update_stats_func = [](const VW::workspace& /* all */, shared_data& sd, const oaa&, const VW::example& ec,
                            VW::io::logger& /* logger */)
    {
      float loss = 0;
      if (ec.l.multi.label != ec.pred.multiclass && ec.l.multi.is_labeled()) { loss = ec.weight; }

      // We specifically say it is not labelled when reporting loss as there is
      // no way to output progressive validation loss for subsampling mode
      // without increasing variance See original fix:
      // https://github.com/VowpalWabbit/vowpal_wabbit/pull/1880
      sd.update(ec.test_only, false, loss, ec.weight, ec.get_num_features());
    };
  }

  auto l = make_reduction_learner(
      std::move(data), base, learn_ptr, pred_ptr, stack_builder.get_setupfn_name(oaa_setup) + name_addition)
               .set_params_per_weight(k_value)
               .set_input_label_type(VW::label_type_t::MULTICLASS)
               .set_output_label_type(VW::label_type_t::SIMPLE)
               .set_input_prediction_type(VW::prediction_type_t::SCALAR)
               .set_output_prediction_type(pred_type)
               .set_update_stats(update_stats_func)
               .set_output_example_prediction(output_example_prediction_func)
               .set_print_update(print_update_func)
               .build();

  return l;
}
