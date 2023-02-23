// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/ftrl.h"

#include "vw/core/correctedMath.h"
#include "vw/core/crossplat_compat.h"
#include "vw/core/label_parser.h"
#include "vw/core/learner.h"
#include "vw/core/loss_functions.h"
#include "vw/core/parse_regressor.h"
#include "vw/core/parser.h"
#include "vw/core/reductions/gd.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/simple_label.h"
#include "vw/io/logger.h"

#include <cfloat>
#include <cmath>
#include <string>

using namespace VW::LEARNER;
using namespace VW::config;
using namespace VW::math;

#define W_XT 0  // current parameter
#define W_ZT 1  // in proximal is "accumulated z(t) = z(t-1) + g(t) + sigma*w(t)", in general is the dual weight vector
#define W_G2 2  // accumulated gradient information
#define W_MX 3  // maximum absolute value
#define W_WE 4  // Wealth
#define W_MG 5  // maximum gradient

namespace
{
class ftrl_update_data
{
public:
  float update = 0.f;
  float ftrl_alpha = 0.f;
  float ftrl_beta = 0.f;
  float l1_lambda = 0.f;
  float l2_lambda = 0.f;
  float predict = 0.f;
  float normalized_squared_norm_x = 0.f;
  float average_squared_norm_x = 0.f;
};

class ftrl
{
public:
  VW::workspace* all = nullptr;  // features, finalize, l1, l2,
  float ftrl_alpha = 0.f;
  float ftrl_beta = 0.f;
  ftrl_update_data data;
  size_t no_win_counter = 0;
  size_t early_stop_thres = 0;
  uint32_t ftrl_size = 0;
  double total_weight = 0.0;
  double normalized_sum_norm_x = 0.0;
};

class uncertainty
{
public:
  float pred;
  float score;
  ftrl& b;
  uncertainty(ftrl& ftrlb) : b(ftrlb)
  {
    pred = 0;
    score = 0;
  }
};

inline void predict_with_confidence(uncertainty& d, const float fx, float& fw)
{
  float* w = &fw;
  d.pred += w[W_XT] * fx;
  float sqrtf_ng2 = sqrtf(w[W_G2]);
  float uncertain = ((d.b.data.ftrl_beta + sqrtf_ng2) / d.b.data.ftrl_alpha + d.b.data.l2_lambda);
  d.score += (1 / uncertain) * sign(fx);
}
float sensitivity(ftrl& b, VW::example& ec)
{
  uncertainty uncetain(b);
  VW::foreach_feature<uncertainty, predict_with_confidence>(*(b.all), ec, uncetain);
  return uncetain.score;
}

template <bool audit>
void predict(ftrl& b, VW::example& ec)
{
  size_t num_features_from_interactions = 0;
  ec.partial_prediction = VW::inline_predict(*b.all, ec, num_features_from_interactions);
  ec.num_features_from_interactions = num_features_from_interactions;
  ec.pred.scalar = VW::details::finalize_prediction(*b.all->sd, b.all->logger, ec.partial_prediction);
  if (audit) { VW::details::print_audit_features(*(b.all), ec); }
}

template <bool audit>
void multipredict(
    ftrl& b, VW::example& ec, size_t count, size_t step, VW::polyprediction* pred, bool finalize_predictions)
{
  VW::workspace& all = *b.all;
  for (size_t c = 0; c < count; c++)
  {
    const auto& simple_red_features = ec.ex_reduction_features.template get<VW::simple_label_reduction_features>();
    pred[c].scalar = simple_red_features.initial;
  }
  size_t num_features_from_interactions = 0;
  if (b.all->weights.sparse)
  {
    VW::details::multipredict_info<VW::sparse_parameters> mp = {
        count, step, pred, all.weights.sparse_weights, static_cast<float>(all.sd->gravity)};
    VW::foreach_feature<VW::details::multipredict_info<VW::sparse_parameters>, uint64_t,
        VW::details::vec_add_multipredict>(all, ec, mp, num_features_from_interactions);
  }
  else
  {
    VW::details::multipredict_info<VW::dense_parameters> mp = {
        count, step, pred, all.weights.dense_weights, static_cast<float>(all.sd->gravity)};
    VW::foreach_feature<VW::details::multipredict_info<VW::dense_parameters>, uint64_t,
        VW::details::vec_add_multipredict>(all, ec, mp, num_features_from_interactions);
  }
  ec.num_features_from_interactions = num_features_from_interactions;
  if (all.sd->contraction != 1.)
  {
    for (size_t c = 0; c < count; c++) { pred[c].scalar *= static_cast<float>(all.sd->contraction); }
  }
  if (finalize_predictions)
  {
    for (size_t c = 0; c < count; c++)
    {
      pred[c].scalar = VW::details::finalize_prediction(*all.sd, all.logger, pred[c].scalar);
    }
  }
  if (audit)
  {
    for (size_t c = 0; c < count; c++)
    {
      ec.pred.scalar = pred[c].scalar;
      VW::details::print_audit_features(all, ec);
      ec.ft_offset += static_cast<uint64_t>(step);
    }
    ec.ft_offset -= static_cast<uint64_t>(step * count);
  }
}

void inner_update_proximal(ftrl_update_data& d, float x, float& wref)
{
  float* w = &wref;
  float gradient = d.update * x;
  float ng2 = w[W_G2] + gradient * gradient;
  float sqrt_ng2 = sqrtf(ng2);
  float sqrt_wW_G2 = sqrtf(w[W_G2]);  // NOLINT
  float sigma = (sqrt_ng2 - sqrt_wW_G2) / d.ftrl_alpha;
  w[W_ZT] += gradient - sigma * w[W_XT];
  w[W_G2] = ng2;
  sqrt_wW_G2 = sqrt_ng2;
  float flag = sign(w[W_ZT]);
  float fabs_zt = w[W_ZT] * flag;
  if (fabs_zt <= d.l1_lambda) { w[W_XT] = 0.; }
  else
  {
    float step = 1 / (d.l2_lambda + (d.ftrl_beta + sqrt_wW_G2) / d.ftrl_alpha);
    w[W_XT] = step * flag * (d.l1_lambda - fabs_zt);
  }
}

void inner_update_pistol_state_and_predict(ftrl_update_data& d, float x, float& wref)
{
  float* w = &wref;

  float fabs_x = std::fabs(x);
  if (fabs_x > w[W_MX]) { w[W_MX] = fabs_x; }

  float squared_theta = w[W_ZT] * w[W_ZT];
  float tmp = 1.f / (d.ftrl_alpha * w[W_MX] * (w[W_G2] + w[W_MX]));
  w[W_XT] = std::sqrt(w[W_G2]) * d.ftrl_beta * w[W_ZT] * VW::details::correctedExp(squared_theta / 2.f * tmp) * tmp;

  d.predict += w[W_XT] * x;
}

void inner_update_pistol_post(ftrl_update_data& d, float x, float& wref)
{
  float* w = &wref;
  float gradient = d.update * x;

  w[W_ZT] += -gradient;
  w[W_G2] += std::fabs(gradient);
}

// Coin betting vectors
// W_XT 0  current parameter
// W_ZT 1  sum negative gradients
// W_G2 2  sum of absolute value of gradients
// W_MX 3  maximum absolute value
// W_WE 4  Wealth
// W_MG 5  Maximum Lipschitz constant
void inner_coin_betting_predict(ftrl_update_data& d, float x, float& wref)
{
  float* w = &wref;
  float w_mx = w[W_MX];
  float w_xt = 0.0;

  float fabs_x = std::fabs(x);
  if (fabs_x > w_mx) { w_mx = fabs_x; }

  // COCOB update without sigmoid
  if (w[W_MG] * w_mx > 0)
  {
    w_xt = ((d.ftrl_alpha + w[W_WE]) / (w[W_MG] * w_mx * (w[W_MG] * w_mx + w[W_G2]))) * w[W_ZT];
  }

  d.predict += w_xt * x;
  if (w_mx > 0)
  {
    const float x_normalized = x / w_mx;
    d.normalized_squared_norm_x += x_normalized * x_normalized;
  }
}

void inner_coin_betting_update_after_prediction(ftrl_update_data& d, float x, float& wref)
{
  float* w = &wref;
  float fabs_x = std::fabs(x);
  float gradient = d.update * x;

  if (fabs_x > w[W_MX]) { w[W_MX] = fabs_x; }

  float fabs_gradient = std::fabs(d.update);
  if (fabs_gradient > w[W_MG]) { w[W_MG] = fabs_gradient > d.ftrl_beta ? fabs_gradient : d.ftrl_beta; }

  // COCOB update without sigmoid.
  // If a new Lipschitz constant and/or magnitude of x is found, the w is
  // recalculated and used in the update of the wealth below.
  if (w[W_MG] * w[W_MX] > 0)
  {
    w[W_XT] = ((d.ftrl_alpha + w[W_WE]) / (w[W_MG] * w[W_MX] * (w[W_MG] * w[W_MX] + w[W_G2]))) * w[W_ZT];
  }
  else { w[W_XT] = 0; }

  w[W_ZT] += -gradient;
  w[W_G2] += std::fabs(gradient);
  w[W_WE] += (-gradient * w[W_XT]);

  w[W_XT] /= d.average_squared_norm_x;
}

void coin_betting_predict(ftrl& b, VW::example& ec)
{
  b.data.predict = 0;
  b.data.normalized_squared_norm_x = 0;

  size_t num_features_from_interactions = 0;
  VW::foreach_feature<ftrl_update_data, inner_coin_betting_predict>(*b.all, ec, b.data, num_features_from_interactions);
  ec.num_features_from_interactions = num_features_from_interactions;

  b.normalized_sum_norm_x += (static_cast<double>(ec.weight)) * b.data.normalized_squared_norm_x;
  b.total_weight += ec.weight;
  b.data.average_squared_norm_x = (static_cast<float>((b.normalized_sum_norm_x + 1e-6) / b.total_weight));

  ec.partial_prediction = b.data.predict / b.data.average_squared_norm_x;

  ec.pred.scalar = VW::details::finalize_prediction(*b.all->sd, b.all->logger, ec.partial_prediction);
}

void update_state_and_predict_pistol(ftrl& b, VW::example& ec)
{
  b.data.predict = 0;

  size_t num_features_from_interactions = 0;
  VW::foreach_feature<ftrl_update_data, inner_update_pistol_state_and_predict>(
      *b.all, ec, b.data, num_features_from_interactions);
  ec.num_features_from_interactions = num_features_from_interactions;

  ec.partial_prediction = b.data.predict;
  ec.pred.scalar = VW::details::finalize_prediction(*b.all->sd, b.all->logger, ec.partial_prediction);
}

void update_after_prediction_proximal(ftrl& b, VW::example& ec)
{
  b.data.update = b.all->loss->first_derivative(b.all->sd.get(), ec.pred.scalar, ec.l.simple.label) * ec.weight;
  VW::foreach_feature<ftrl_update_data, inner_update_proximal>(*b.all, ec, b.data);
}

void update_after_prediction_pistol(ftrl& b, VW::example& ec)
{
  b.data.update = b.all->loss->first_derivative(b.all->sd.get(), ec.pred.scalar, ec.l.simple.label) * ec.weight;
  VW::foreach_feature<ftrl_update_data, inner_update_pistol_post>(*b.all, ec, b.data);
}

void coin_betting_update_after_prediction(ftrl& b, VW::example& ec)
{
  b.data.update = b.all->loss->first_derivative(b.all->sd.get(), ec.pred.scalar, ec.l.simple.label) * ec.weight;
  VW::foreach_feature<ftrl_update_data, inner_coin_betting_update_after_prediction>(*b.all, ec, b.data);
}

template <bool audit>
void learn_proximal(ftrl& a, VW::example& ec)
{
  // predict with confidence
  predict<audit>(a, ec);

  // update state based on the prediction
  update_after_prediction_proximal(a, ec);
}

template <bool audit>
void learn_pistol(ftrl& a, VW::example& ec)
{
  // update state based on the example and predict
  update_state_and_predict_pistol(a, ec);
  if (audit) { VW::details::print_audit_features(*(a.all), ec); }
  // update state based on the prediction
  update_after_prediction_pistol(a, ec);
}

template <bool audit>
void learn_coin_betting(ftrl& a, VW::example& ec)
{
  // update state based on the example and predict
  coin_betting_predict(a, ec);
  if (audit) { VW::details::print_audit_features(*(a.all), ec); }
  // update state based on the prediction
  coin_betting_update_after_prediction(a, ec);
}

void save_load(ftrl& b, VW::io_buf& model_file, bool read, bool text)
{
  VW::workspace* all = b.all;
  if (read) { VW::details::initialize_regressor(*all); }

  if (model_file.num_files() != 0)
  {
    bool resume = all->save_resume;
    std::stringstream msg;
    msg << ":" << resume << "\n";
    VW::details::bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&resume), sizeof(resume), read, msg, text);

    if (resume)
    {
      VW::details::save_load_online_state_gd(
          *all, model_file, read, text, b.total_weight, b.normalized_sum_norm_x, nullptr, b.ftrl_size);
    }
    else { VW::details::save_load_regressor_gd(*all, model_file, read, text); }
  }
}

void end_pass(ftrl& g)
{
  VW::workspace& all = *g.all;

  if (!all.holdout_set_off)
  {
    if (VW::details::summarize_holdout_set(all, g.no_win_counter))
    {
      VW::details::finalize_regressor(all, all.final_regressor_name);
    }
    if ((g.early_stop_thres == g.no_win_counter) &&
        ((all.check_holdout_every_n_passes <= 1) || ((all.current_pass % all.check_holdout_every_n_passes) == 0)))
    {
      VW::details::set_done(all);
    }
  }
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::ftrl_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto b = VW::make_unique<ftrl>();

  bool ftrl_option_no_not_use = false;
  bool pistol_no_not_use = false;
  bool coin_no_not_use = false;

  option_group_definition ftrl_options("[Reduction] Follow the Regularized Leader - FTRL");
  ftrl_options
      .add(make_option("ftrl", ftrl_option_no_not_use)
               .necessary()
               .keep()
               .help("FTRL: Follow the Proximal Regularized Leader"))
      .add(make_option("ftrl_alpha", b->ftrl_alpha).help("Learning rate for FTRL optimization"))
      .add(make_option("ftrl_beta", b->ftrl_beta).help("Learning rate for FTRL optimization"));

  option_group_definition pistol_options("[Reduction] Follow the Regularized Leader - Pistol");
  pistol_options
      .add(make_option("pistol", pistol_no_not_use)
               .necessary()
               .keep()
               .help("PiSTOL: Parameter-free STOchastic Learning"))
      .add(make_option("ftrl_alpha", b->ftrl_alpha).help("Learning rate for FTRL optimization"))
      .add(make_option("ftrl_beta", b->ftrl_beta).help("Learning rate for FTRL optimization"));

  option_group_definition coin_options("[Reduction] Follow the Regularized Leader - Coin");
  coin_options.add(make_option("coin", coin_no_not_use).necessary().keep().help("Coin betting optimizer"))
      .add(make_option("ftrl_alpha", b->ftrl_alpha).help("Learning rate for FTRL optimization"))
      .add(make_option("ftrl_beta", b->ftrl_beta).help("Learning rate for FTRL optimization"));

  const auto ftrl_enabled = options.add_parse_and_check_necessary(ftrl_options);
  const auto pistol_enabled = options.add_parse_and_check_necessary(pistol_options);
  const auto coin_enabled = options.add_parse_and_check_necessary(coin_options);

  if (!ftrl_enabled && !pistol_enabled && !coin_enabled) { return nullptr; }
  size_t count = 0;
  count += ftrl_enabled ? 1 : 0;
  count += pistol_enabled ? 1 : 0;
  count += coin_enabled ? 1 : 0;

  if (count != 1) { THROW("You can only use one of 'ftrl', 'pistol', or 'coin' at a time."); }

  b->all = &all;
  b->no_win_counter = 0;
  b->normalized_sum_norm_x = 0;
  b->total_weight = 0;

  std::string algorithm_name;
  void (*learn_ptr)(ftrl&, VW::example&) = nullptr;
  bool learn_returns_prediction = false;

  // Defaults that are specific to the mode that was chosen.
  if (ftrl_enabled)
  {
    b->ftrl_alpha = options.was_supplied("ftrl_alpha") ? b->ftrl_alpha : 0.005f;
    b->ftrl_beta = options.was_supplied("ftrl_beta") ? b->ftrl_beta : 0.1f;
    algorithm_name = "Proximal-FTRL";
    learn_ptr = all.audit || all.hash_inv ? learn_proximal<true> : learn_proximal<false>;
    all.weights.stride_shift(2);  // NOTE: for more parameter storage
    b->ftrl_size = 3;
  }
  else if (pistol_enabled)
  {
    b->ftrl_alpha = options.was_supplied("ftrl_alpha") ? b->ftrl_alpha : 1.0f;
    b->ftrl_beta = options.was_supplied("ftrl_beta") ? b->ftrl_beta : 0.5f;
    algorithm_name = "PiSTOL";
    learn_ptr = all.audit || all.hash_inv ? learn_pistol<true> : learn_pistol<false>;
    all.weights.stride_shift(2);  // NOTE: for more parameter storage
    b->ftrl_size = 4;
    learn_returns_prediction = true;
  }
  else if (coin_enabled)
  {
    b->ftrl_alpha = options.was_supplied("ftrl_alpha") ? b->ftrl_alpha : 4.0f;
    b->ftrl_beta = options.was_supplied("ftrl_beta") ? b->ftrl_beta : 1.0f;
    algorithm_name = "Coin Betting";
    learn_ptr = all.audit || all.hash_inv ? learn_coin_betting<true> : learn_coin_betting<false>;
    all.weights.stride_shift(3);  // NOTE: for more parameter storage
    b->ftrl_size = 6;
    learn_returns_prediction = true;
  }

  b->data.ftrl_alpha = b->ftrl_alpha;
  b->data.ftrl_beta = b->ftrl_beta;
  b->data.l1_lambda = b->all->l1_lambda;
  b->data.l2_lambda = b->all->l2_lambda;

  if (!all.quiet)
  {
    *(all.trace_message) << "Enabling FTRL based optimization" << std::endl;
    *(all.trace_message) << "Algorithm used: " << algorithm_name << std::endl;
    *(all.trace_message) << "ftrl_alpha = " << b->ftrl_alpha << std::endl;
    *(all.trace_message) << "ftrl_beta = " << b->ftrl_beta << std::endl;
  }

  if (!all.holdout_set_off)
  {
    all.sd->holdout_best_loss = FLT_MAX;
    b->early_stop_thres = options.get_typed_option<uint64_t>("early_terminate").value();
  }

  auto predict_ptr = (all.audit || all.hash_inv) ? predict<true> : predict<false>;
  auto multipredict_ptr = (all.audit || all.hash_inv) ? multipredict<true> : multipredict<false>;
  std::string name_addition = (all.audit || all.hash_inv) ? "-audit" : "";

  auto l = VW::LEARNER::make_bottom_learner(std::move(b), learn_ptr, predict_ptr,
      stack_builder.get_setupfn_name(ftrl_setup) + "-" + algorithm_name + name_addition, VW::prediction_type_t::SCALAR,
      VW::label_type_t::SIMPLE)
               .set_learn_returns_prediction(learn_returns_prediction)
               .set_params_per_weight(VW::details::UINT64_ONE << all.weights.stride_shift())
               .set_sensitivity(sensitivity)
               .set_multipredict(multipredict_ptr)
               .set_save_load(save_load)
               .set_end_pass(end_pass)
               .set_output_example_prediction(VW::details::output_example_prediction_simple_label<ftrl>)
               .set_update_stats(VW::details::update_stats_simple_label<ftrl>)
               .set_print_update(VW::details::print_update_simple_label<ftrl>)
               .build();
  return l;
}