// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "active.h"

#include "config/options.h"
#include "io/logger.h"
#include "model_utils.h"
#include "setup_base.h"
#include "shared_data.h"
#include "vw.h"
#include "vw_exception.h"
#include "vw_math.h"
#include "vw_string_view.h"
#include "vw_versions.h"

#include <cerrno>
#include <cfloat>
#include <cmath>

using namespace VW::LEARNER;
using namespace VW::config;
using namespace VW::reductions;

float get_active_coin_bias(float k, float avg_loss, float g, float c0)
{
  const float b = c0 * (std::log(k + 1.f) + 0.0001f) / (k + 0.0001f);
  const float sb = std::sqrt(b);
  // loss should be in [0,1]
  avg_loss = VW::math::clamp(avg_loss, 0.f, 1.f);

  const float sl = std::sqrt(avg_loss) + std::sqrt(avg_loss + g);
  if (g <= sb * sl + b) { return 1; }
  const float rs = (sl + std::sqrt(sl * sl + 4 * g)) / (2 * g);
  return b * rs * rs;
}

float query_decision(active& a, float ec_revert_weight, float k)
{
  float bias;
  if (k <= 1.f)
    bias = 1.f;
  else
  {
    const auto weighted_queries = static_cast<float>(a._shared_data->weighted_labeled_examples);
    const float avg_loss = (static_cast<float>(a._shared_data->sum_loss) / k) +
        std::sqrt((1.f + 0.5f * std::log(k)) / (weighted_queries + 0.0001f));
    bias = get_active_coin_bias(k, avg_loss, ec_revert_weight / k, a.active_c0);
  }

  return (a._random_state->get_and_update_random() < bias) ? 1.f / bias : -1.f;
}

template <bool is_learn>
void predict_or_learn_simulation(active& a, single_learner& base, VW::example& ec)
{
  base.predict(ec);

  if (is_learn)
  {
    const auto k = static_cast<float>(a._shared_data->t);
    constexpr float threshold = 0.f;

    ec.confidence = fabsf(ec.pred.scalar - threshold) / base.sensitivity(ec);
    const float importance = query_decision(a, ec.confidence, k);

    if (importance > 0.f)
    {
      a._shared_data->queries += 1;
      ec.weight *= importance;
      base.learn(ec);
    }
    else
    {
      ec.l.simple.label = FLT_MAX;
      ec.weight = 0.f;
    }
  }
}

template <bool is_learn>
void predict_or_learn_active(active& a, single_learner& base, VW::example& ec)
{
  if (is_learn)
    base.learn(ec);
  else
    base.predict(ec);

  if (ec.l.simple.label == FLT_MAX)
  {
    const float threshold = (a._shared_data->max_label + a._shared_data->min_label) * 0.5f;
    // We want to understand the change in prediction if the label were to be
    // the opposite of what was predicted. 0 and 1 are used for the expected min
    // and max labels to be coming in from the active interactor.
    ec.l.simple.label = (ec.pred.scalar >= threshold) ? a._min_seen_label : a._max_seen_label;
    ec.confidence = std::abs(ec.pred.scalar - threshold) / base.sensitivity(ec);
    ec.l.simple.label = FLT_MAX;
  }
  else
  {
    // Update seen labels based on the current example's label.
    a._min_seen_label = std::min(ec.l.simple.label, a._min_seen_label);
    a._max_seen_label = std::max(ec.l.simple.label, a._max_seen_label);
  }
}

void active_print_result(
    VW::io::writer* f, float res, float weight, const VW::v_array<char>& tag, VW::io::logger& logger)
{
  if (f == nullptr) { return; }

  std::stringstream ss;
  ss << std::fixed << res;
  ss << " ";
  if (!tag.empty()) { ss << VW::string_view{tag.begin(), tag.size()}; }
  if (weight >= 0) { ss << " " << std::fixed << weight; }
  ss << '\n';
  const auto ss_str = ss.str();
  ssize_t len = ss_str.size();
  ssize_t t = f->write(ss_str.c_str(), static_cast<unsigned int>(len));
  if (t != len) { logger.err_error("write error: {}", VW::strerror_to_string(errno)); }
}

void output_and_account_example(VW::workspace& all, active& a, VW::example& ec)
{
  const label_data& ld = ec.l.simple;

  all.sd->update(ec.test_only, ld.label != FLT_MAX, ec.loss, ec.weight, ec.get_num_features());
  if (ld.label != FLT_MAX && !ec.test_only)
  { all.sd->weighted_labels += (static_cast<double>(ld.label)) * static_cast<double>(ec.weight); }

  float ai = -1;
  if (ld.label == FLT_MAX)
  { ai = query_decision(a, ec.confidence, static_cast<float>(all.sd->weighted_unlabeled_examples)); }

  all.print_by_ref(all.raw_prediction.get(), ec.partial_prediction, -1, ec.tag, all.logger);
  for (auto& i : all.final_prediction_sink) { active_print_result(i.get(), ec.pred.scalar, ai, ec.tag, all.logger); }

  print_update(all, ec);
}

template <bool simulation>
void return_active_example(VW::workspace& all, active& a, VW::example& ec)
{
  if (simulation) { output_and_account_example(all, ec); }
  else
  {
    output_and_account_example(all, a, ec);
  }
  VW::finish_example(all, ec);
}

void save_load(active& a, io_buf& io, bool read, bool text)
{
  if (io.num_files() == 0) { return; }
  if (a._model_version >= VW::version_definitions::VERSION_FILE_WITH_ACTIVE_SEEN_LABELS)
  {
    if (read)
    {
      VW::model_utils::read_model_field(io, a._min_seen_label);
      VW::model_utils::read_model_field(io, a._max_seen_label);
    }
    else
    {
      VW::model_utils::write_model_field(io, a._min_seen_label, "Active: min_seen_label {}", text);
      VW::model_utils::write_model_field(io, a._max_seen_label, "Active: max_seen_label {}", text);
    }
  }
}

base_learner* VW::reductions::active_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();

  bool active_option = false;
  bool simulation = false;
  float active_c0;
  option_group_definition new_options("[Reduction] Active Learning");
  new_options.add(make_option("active", active_option).keep().necessary().help("Enable active learning"))
      .add(make_option("simulation", simulation).help("Active learning simulation mode"))
      .add(make_option("mellowness", active_c0)
               .keep()
               .default_value(8.f)
               .help("Active learning mellowness parameter c_0. Default 8"));

  if (!options.add_parse_and_check_necessary(new_options)) return nullptr;

  if (options.was_supplied("lda")) { THROW("lda cannot be combined with active learning") }
  auto data = VW::make_unique<active>(active_c0, all.sd, all.get_random_state(), all.model_file_ver);
  auto base = as_singleline(stack_builder.setup_base_learner());

  using learn_pred_func_t = void (*)(active&, VW::LEARNER::single_learner&, VW::example&);
  learn_pred_func_t learn_func;
  learn_pred_func_t pred_func;
  void (*finish_ptr)(VW::workspace&, active&, VW::example&);
  bool learn_returns_prediction = true;
  std::string reduction_name = stack_builder.get_setupfn_name(VW::reductions::active_setup);
  if (simulation)
  {
    learn_func = predict_or_learn_simulation<true>;
    pred_func = predict_or_learn_simulation<false>;
    finish_ptr = return_active_example<true>;
    reduction_name.append("-simulation");
  }
  else
  {
    all.active = true;
    learn_func = predict_or_learn_active<true>;
    pred_func = predict_or_learn_active<false>;
    finish_ptr = return_active_example<false>;
    learn_returns_prediction = base->learn_returns_prediction;
  }

  // Create new learner
  auto* l = make_reduction_learner(std::move(data), base, learn_func, pred_func, reduction_name)
                .set_input_label_type(VW::label_type_t::simple)
                .set_output_prediction_type(VW::prediction_type_t::scalar)
                .set_learn_returns_prediction(learn_returns_prediction)
                .set_save_load(save_load)
                .set_finish_example(finish_ptr)
                .build();

  return make_base(*l);
}
