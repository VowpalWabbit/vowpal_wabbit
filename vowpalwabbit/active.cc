// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cerrno>
#include <cfloat>
#include <cmath>

#include "reductions.h"
#include "vw.h"
#include "active.h"
#include "vw_exception.h"
#include "shared_data.h"

#include "io/logger.h"

using namespace VW::LEARNER;
using namespace VW::config;

namespace logger = VW::io::logger;

float get_active_coin_bias(float k, float avg_loss, float g, float c0)
{
  const float b = c0 * (std::log(k + 1.f) + 0.0001f) / (k + 0.0001f);
  const float sb = std::sqrt(b);
  avg_loss = std::min(1.f, std::max(0.f, avg_loss));  // loss should be in [0,1]

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
void predict_or_learn_simulation(active& a, single_learner& base, example& ec)
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
void predict_or_learn_active(active& a, single_learner& base, example& ec)
{
  if (is_learn)
    base.learn(ec);
  else
    base.predict(ec);

  if (ec.l.simple.label == FLT_MAX)
  {
    const float threshold = (a._shared_data->max_label + a._shared_data->min_label) * 0.5f;
    ec.confidence = fabsf(ec.pred.scalar - threshold) / base.sensitivity(ec);
  }
}

void active_print_result(VW::io::writer* f, float res, float weight, const v_array<char>& tag)
{
  if (f == nullptr) { return; }
  const auto weight_str = weight >= 0 ? fmt::format(" {:f}", weight) : "";
  const auto tag_str = !tag.empty() ? fmt::format(" {}", std::string(tag.begin(), tag.end())) : "";
  const auto result = fmt::format("{:f}{}{}\n", res, tag_str, weight_str);
  const auto t = f->write(result.c_str(), result.size());
  if (t != static_cast<ssize_t>(result.size()))
  { logger::errlog_error("write error: {}", VW::strerror_to_string(errno)); }
}

void output_and_account_example(vw& all, active& a, example& ec)
{
  const label_data& ld = ec.l.simple;

  all.sd->update(ec.test_only, ld.label != FLT_MAX, ec.loss, ec.weight, ec.get_num_features());
  if (ld.label != FLT_MAX && !ec.test_only)
  { all.sd->weighted_labels += (static_cast<double>(ld.label)) * static_cast<double>(ec.weight); }
  all.sd->weighted_unlabeled_examples += ld.label == FLT_MAX ? static_cast<double>(ec.weight) : 0.0;

  float ai = -1;
  if (ld.label == FLT_MAX)
  { ai = query_decision(a, ec.confidence, static_cast<float>(all.sd->weighted_unlabeled_examples)); }

  all.print_by_ref(all.raw_prediction.get(), ec.partial_prediction, -1, ec.tag);
  for (auto& i : all.final_prediction_sink) { active_print_result(i.get(), ec.pred.scalar, ai, ec.tag); }

  print_update(all, ec);
}

void return_active_example(vw& all, active& a, example& ec)
{
  output_and_account_example(all, a, ec);
  VW::finish_example(all, ec);
}

base_learner* active_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  vw& all = *stack_builder.get_all_pointer();

  bool active_option = false;
  bool simulation = false;
  float active_c0;
  option_group_definition new_options("Active Learning");
  new_options.add(make_option("active", active_option).keep().necessary().help("enable active learning"))
      .add(make_option("simulation", simulation).help("active learning simulation mode"))
      .add(make_option("mellowness", active_c0)
               .keep()
               .default_value(8.f)
               .help("active learning mellowness parameter c_0. Default 8"));

  if (!options.add_parse_and_check_necessary(new_options)) return nullptr;

  if (options.was_supplied("lda")) { THROW("error: you can't combine lda and active learning") }
  auto data = VW::make_unique<active>(active_c0, all.sd, all.get_random_state());
  auto base = as_singleline(stack_builder.setup_base_learner());

  using learn_pred_func_t = void (*)(active&, VW::LEARNER::single_learner&, example&);
  learn_pred_func_t learn_func;
  learn_pred_func_t pred_func;
  bool learn_returns_prediction = true;
  std::string reduction_name = stack_builder.get_setupfn_name(active_setup);
  if (simulation)
  {
    learn_func = predict_or_learn_simulation<true>;
    pred_func = predict_or_learn_simulation<false>;
    reduction_name.append("-simulation");
  }
  else
  {
    learn_func = predict_or_learn_active<true>;
    pred_func = predict_or_learn_active<false>;
    learn_returns_prediction = base->learn_returns_prediction;
  }

  // Create new learner
  auto reduction_builder = make_reduction_learner(std::move(data), base, learn_func, pred_func, reduction_name);
  reduction_builder.set_label_type(label_type_t::simple);
  reduction_builder.set_prediction_type(prediction_type_t::scalar);
  reduction_builder.set_learn_returns_prediction(learn_returns_prediction);
  if (!simulation) { reduction_builder.set_finish_example(return_active_example); }

  return make_base(*reduction_builder.build());
}
