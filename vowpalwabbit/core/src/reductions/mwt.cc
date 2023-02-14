// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/mwt.h"

#include "vw/config/options.h"
#include "vw/core/cb.h"
#include "vw/core/constant.h"
#include "vw/core/io_buf.h"
#include "vw/core/reductions/cb/cb_algs.h"
#include "vw/core/reductions/gd.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/io/errno_handling.h"
#include "vw/io/logger.h"

#include <cmath>

using namespace VW::LEARNER;
using namespace VW::config;

void VW::details::print_scalars(
    VW::io::writer* f, const VW::v_array<float>& scalars, const VW::v_array<char>& tag, VW::io::logger& logger)
{
  if (f != nullptr)
  {
    std::stringstream ss;

    for (size_t i = 0; i < scalars.size(); i++)
    {
      if (i > 0) { ss << ' '; }
      ss << scalars[i];
    }
    for (size_t i = 0; i < tag.size(); i++)
    {
      if (i == 0) { ss << ' '; }
      ss << tag[i];
    }
    ss << '\n';
    ssize_t len = ss.str().size();
    ssize_t t = f->write(ss.str().c_str(), static_cast<unsigned int>(len));
    if (t != len) { logger.err_error("write error: {}", VW::io::strerror_to_string(errno)); }
  }
}

namespace
{
class policy_data
{
public:
  double cost = 0.0;
  uint32_t action = 0;
  bool seen = false;
};

class mwt
{
public:
  std::array<bool, VW::NUM_NAMESPACES> namespaces{};  // the set of namespaces to evaluate.
  std::vector<policy_data> evals;                     // accrued losses of features.
  std::pair<bool, VW::cb_class> optional_observation;
  VW::v_array<uint64_t> policies;
  double total = 0.;
  uint32_t num_classes = 0;
  bool learn = false;

  VW::v_array<VW::namespace_index> indices;  // excluded namespaces
  std::array<VW::features, VW::NUM_NAMESPACES> feature_space;
  VW::workspace* all = nullptr;
};

void value_policy(mwt& c, float val, uint64_t index)  // estimate the value of a single feature.
{
  if (val < 0 || std::floor(val) != val) { c.all->logger.out_error("error {} is not a valid action", val); }

  auto value = static_cast<uint32_t>(val);
  uint64_t new_index = (index & c.all->weights.mask()) >> c.all->weights.stride_shift();

  if (!c.evals[new_index].seen)
  {
    c.evals[new_index].seen = true;
    c.policies.push_back(new_index);
  }

  c.evals[new_index].action = value;
}

template <bool learn, bool exclude, bool is_learn>
void predict_or_learn(mwt& c, learner& base, VW::example& ec)
{
  c.optional_observation = get_observed_cost_cb(ec.l.cb);

  if (c.optional_observation.first)
  {
    c.total++;
    // For each nonzero feature in observed namespaces, check it's value.
    for (unsigned char ns : ec.indices)
    {
      if (c.namespaces[ns]) { VW::foreach_feature<mwt, value_policy>(c.all, ec.feature_space[ns], c); }
    }
    for (uint64_t policy : c.policies)
    {
      c.evals[policy].cost += get_cost_estimate(c.optional_observation.second, c.evals[policy].action);
      c.evals[policy].action = 0;
    }
  }

  VW_WARNING_STATE_PUSH
  VW_WARNING_DISABLE_COND_CONST_EXPR
  if VW_STD17_CONSTEXPR (exclude || learn)
  {
    c.indices.clear();
    uint32_t stride_shift = c.all->weights.stride_shift();
    uint64_t weight_mask = c.all->weights.mask();
    for (unsigned char ns : ec.indices)
    {
      if (c.namespaces[ns])
      {
        c.indices.push_back(ns);
        if (learn)
        {
          c.feature_space[ns].clear();
          for (VW::features::iterator& f : ec.feature_space[ns])
          {
            uint64_t new_index =
                ((f.index() & weight_mask) >> stride_shift) * c.num_classes + static_cast<uint64_t>(f.value());
            c.feature_space[ns].push_back(1, new_index << stride_shift);
          }
        }
        std::swap(c.feature_space[ns], ec.feature_space[ns]);
      }
    }
  }
  VW_WARNING_STATE_POP

  // modify the predictions to use a vector with a score for each evaluated feature.
  VW::v_array<float> preds = ec.pred.scalars;

  if (learn)
  {
    if (is_learn) { base.learn(ec); }
    else { base.predict(ec); }
  }

  VW_WARNING_STATE_PUSH
  VW_WARNING_DISABLE_COND_CONST_EXPR
  if VW_STD17_CONSTEXPR (exclude || learn)
  {
    while (!c.indices.empty())
    {
      unsigned char ns = c.indices.back();
      c.indices.pop_back();
      std::swap(c.feature_space[ns], ec.feature_space[ns]);
    }
  }
  VW_WARNING_STATE_POP

  // modify the predictions to use a vector with a score for each evaluated feature.
  preds.clear();
  if (learn) { preds.push_back(static_cast<float>(ec.pred.multiclass)); }
  for (uint64_t index : c.policies)
  {
    preds.push_back(static_cast<float>(c.evals[index].cost) / static_cast<float>(c.total));
  }

  ec.pred.scalars = preds;
}

void update_stats_mwt(const VW::workspace& /* all */, VW::shared_data& sd, const mwt& data, const VW::example& ec,
    VW::io::logger& /* logger */)
{
  float loss = 0.;
  if (data.learn)
  {
    if (data.optional_observation.first)
    {
      loss = get_cost_estimate(data.optional_observation.second, static_cast<uint32_t>(ec.pred.scalars[0]));
    }
  }
  sd.update(ec.test_only, data.optional_observation.first, loss, 1.f, ec.get_num_features());
}

void output_example_prediction_mwt(
    VW::workspace& all, const mwt& /* data */, const VW::example& ec, VW::io::logger& /* unused */)
{
  for (auto& sink : all.final_prediction_sink)
  {
    VW::details::print_scalars(sink.get(), ec.pred.scalars, ec.tag, all.logger);
  }
}

void print_update_mwt(
    VW::workspace& all, VW::shared_data& /* sd */, const mwt& data, const VW::example& ec, VW::io::logger& /* unused */)
{
  const bool should_print_driver_update =
      all.sd->weighted_examples() >= all.sd->dump_interval && !all.quiet && !all.bfgs;

  if (should_print_driver_update && data.learn)
  {
    size_t num_features = ec.get_num_features();
    size_t pred = ec.pred.multiclass;

    std::string label_buf;
    if (data.optional_observation.first) { label_buf = "unknown"; }
    else { label_buf = " known"; }

    all.sd->print_update(*all.trace_message, all.holdout_set_off, all.current_pass, label_buf,
        static_cast<uint32_t>(pred), num_features);
  }
}

void save_load(mwt& c, VW::io_buf& model_file, bool read, bool text)
{
  if (model_file.num_files() == 0) { return; }

  std::stringstream msg;

  // total
  msg << "total: " << c.total;
  VW::details::bin_text_read_write_fixed_validated(
      model_file, reinterpret_cast<char*>(&c.total), sizeof(c.total), read, msg, text);

  // policies
  size_t policies_size = c.policies.size();
  VW::details::bin_text_read_write_fixed_validated(
      model_file, reinterpret_cast<char*>(&policies_size), sizeof(policies_size), read, msg, text);

  if (read) { c.policies.resize(policies_size); }
  else
  {
    msg << "policies: ";
    for (VW::feature_index& policy : c.policies) { msg << policy << " "; }
  }

  VW::details::bin_text_read_write_fixed_validated(model_file, reinterpret_cast<char*>(c.policies.begin()),
      policies_size * sizeof(VW::feature_index), read, msg, text);

  // c.evals is already initialized nicely to the same size as the regressor.
  for (VW::feature_index& policy : c.policies)
  {
    policy_data& pd = c.evals[policy];
    if (read) { msg << "evals: " << policy << ":" << pd.action << ":" << pd.cost << " "; }
    VW::details::bin_text_read_write_fixed_validated(
        model_file, reinterpret_cast<char*>(&c.evals[policy].cost), sizeof(double), read, msg, text);
    VW::details::bin_text_read_write_fixed_validated(
        model_file, reinterpret_cast<char*>(&c.evals[policy].action), sizeof(uint32_t), read, msg, text);
    VW::details::bin_text_read_write_fixed_validated(
        model_file, reinterpret_cast<char*>(&c.evals[policy].seen), sizeof(bool), read, msg, text);
  }
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::mwt_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto c = VW::make_unique<mwt>();
  std::string s;
  bool exclude_eval = false;
  option_group_definition new_options("[Reduction] Multiworld Testing");
  new_options.add(make_option("multiworld_test", s).keep().necessary().help("Evaluate features as a policies"))
      .add(make_option("learn", c->num_classes).help("Do Contextual Bandit learning on <n> classes"))
      .add(make_option("exclude_eval", exclude_eval).help("Discard mwt policy features before learning"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  for (char i : s) { c->namespaces[static_cast<unsigned char>(i)] = true; }
  c->all = &all;

  c->evals.resize(all.length(), policy_data{});

  bool cb_added = false;
  if (c->num_classes > 0)
  {
    c->learn = true;

    if (!options.was_supplied("cb"))
    {
      std::stringstream ss;
      ss << c->num_classes;
      options.insert("cb", ss.str());
      cb_added = true;
    }
  }

  if (options.was_supplied("cb") || cb_added)
  {
    // default to legacy cb implementation
    options.insert("cb_force_legacy", "");
  }

  std::string name_addition;
  void (*learn_ptr)(mwt&, learner&, VW::example&) = nullptr;
  void (*pred_ptr)(mwt&, learner&, VW::example&) = nullptr;

  if (c->learn)
  {
    if (exclude_eval)
    {
      name_addition = "-no_eval";
      learn_ptr = predict_or_learn<true, true, true>;
      pred_ptr = predict_or_learn<true, true, false>;
    }
    else
    {
      name_addition = "-eval";
      learn_ptr = predict_or_learn<true, false, true>;
      pred_ptr = predict_or_learn<true, false, false>;
    }
  }
  else
  {
    name_addition = "";
    learn_ptr = predict_or_learn<false, false, true>;
    pred_ptr = predict_or_learn<false, false, false>;
  }

  auto base = stack_builder.setup_base_learner();

  auto l = make_reduction_learner(std::move(c), require_singleline(base), learn_ptr, pred_ptr,
      stack_builder.get_setupfn_name(mwt_setup) + name_addition)
               .set_learn_returns_prediction(true)
               .set_input_prediction_type(base->get_output_prediction_type())
               .set_output_prediction_type(VW::prediction_type_t::SCALARS)
               .set_input_label_type(VW::label_type_t::CB)
               .set_output_label_type(base->get_input_label_type())
               .set_save_load(save_load)
               .set_output_example_prediction(::output_example_prediction_mwt)
               .set_update_stats(::update_stats_mwt)
               .set_print_update(::print_update_mwt)
               .build();

  all.example_parser->lbl_parser = VW::cb_label_parser_global;
  return l;
}
