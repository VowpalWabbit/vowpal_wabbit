/*
   Copyright (c) by respective owners including Yahoo!, Microsoft, and
   individual contributors. All rights reserved.  Released under a BSD (revised)
   license as described in the file LICENSE.
   */
#include <string>
#include "correctedMath.h"
#include "gd.h"

using namespace LEARNER;
using namespace VW::config;

#define W_XT 0  // current parameter
#define W_ZT 1  // in proximal is "accumulated z(t) = z(t-1) + g(t) + sigma*w(t)", in general is the dual weight vector
#define W_G2 2  // accumulated gradient information
#define W_MX 3  // maximum absolute value
#define W_WE 4  // Wealth
#define W_MG 5  // maximum gradient

struct update_data
{
  float update;
  float ftrl_alpha;
  float ftrl_beta;
  float l1_lambda;
  float l2_lambda;
  float predict;
  float normalized_squared_norm_x;
};

struct ftrl
{
  vw* all;  // features, finalize, l1, l2,
  float ftrl_alpha;
  float ftrl_beta;
  struct update_data data;
  size_t no_win_counter;
  size_t early_stop_thres;
  uint32_t ftrl_size;
  double total_weight;
};

struct uncertainty
{
  float pred;
  float score;
  ftrl& b;
  uncertainty(ftrl& ftrlb) : b(ftrlb)
  {
    pred = 0;
    score = 0;
  }
};

inline float sign(float w)
{
  if (w < 0.)
    return -1.;
  else
    return 1.;
}

inline void predict_with_confidence(uncertainty& d, const float fx, float& fw)
{
  float* w = &fw;
  d.pred += w[W_XT] * fx;
  float sqrtf_ng2 = sqrtf(w[W_G2]);
  float uncertain = ((d.b.data.ftrl_beta + sqrtf_ng2) / d.b.data.ftrl_alpha + d.b.data.l2_lambda);
  d.score += (1 / uncertain) * sign(fx);
}
float sensitivity(ftrl& b, base_learner& /* base */, example& ec)
{
  uncertainty uncetain(b);
  GD::foreach_feature<uncertainty, predict_with_confidence>(*(b.all), ec, uncetain);
  return uncetain.score;
}

template <bool audit>
void predict(ftrl& b, single_learner&, example& ec)
{
  ec.partial_prediction = GD::inline_predict(*b.all, ec);
  ec.pred.scalar = GD::finalize_prediction(b.all->sd, ec.partial_prediction);
  if (audit)
    GD::print_audit_features(*(b.all), ec);
}

template <bool audit>
void multipredict(
    ftrl& b, base_learner&, example& ec, size_t count, size_t step, polyprediction* pred, bool finalize_predictions)
{
  vw& all = *b.all;
  for (size_t c = 0; c < count; c++) pred[c].scalar = ec.l.simple.initial;
  if (b.all->weights.sparse)
  {
    GD::multipredict_info<sparse_parameters> mp = {
        count, step, pred, all.weights.sparse_weights, (float)all.sd->gravity};
    GD::foreach_feature<GD::multipredict_info<sparse_parameters>, uint64_t, GD::vec_add_multipredict>(all, ec, mp);
  }
  else
  {
    GD::multipredict_info<dense_parameters> mp = {count, step, pred, all.weights.dense_weights, (float)all.sd->gravity};
    GD::foreach_feature<GD::multipredict_info<dense_parameters>, uint64_t, GD::vec_add_multipredict>(all, ec, mp);
  }
  if (all.sd->contraction != 1.)
    for (size_t c = 0; c < count; c++) pred[c].scalar *= (float)all.sd->contraction;
  if (finalize_predictions)
    for (size_t c = 0; c < count; c++) pred[c].scalar = GD::finalize_prediction(all.sd, pred[c].scalar);
  if (audit)
  {
    for (size_t c = 0; c < count; c++)
    {
      ec.pred.scalar = pred[c].scalar;
      GD::print_audit_features(all, ec);
      ec.ft_offset += (uint64_t)step;
    }
    ec.ft_offset -= (uint64_t)(step * count);
  }
}

void inner_update_proximal(update_data& d, float x, float& wref)
{
  float* w = &wref;
  float gradient = d.update * x;
  float ng2 = w[W_G2] + gradient * gradient;
  float sqrt_ng2 = sqrtf(ng2);
  float sqrt_wW_G2 = sqrtf(w[W_G2]);
  float sigma = (sqrt_ng2 - sqrt_wW_G2) / d.ftrl_alpha;
  w[W_ZT] += gradient - sigma * w[W_XT];
  w[W_G2] = ng2;
  sqrt_wW_G2 = sqrt_ng2;
  float flag = sign(w[W_ZT]);
  float fabs_zt = w[W_ZT] * flag;
  if (fabs_zt <= d.l1_lambda)
    w[W_XT] = 0.;
  else
  {
    float step = 1 / (d.l2_lambda + (d.ftrl_beta + sqrt_wW_G2) / d.ftrl_alpha);
    w[W_XT] = step * flag * (d.l1_lambda - fabs_zt);
  }
}

void inner_update_pistol_state_and_predict(update_data& d, float x, float& wref)
{
  float* w = &wref;

  float fabs_x = fabs(x);
  if (fabs_x > w[W_MX])
    w[W_MX] = fabs_x;

  float squared_theta = w[W_ZT] * w[W_ZT];
  float tmp = 1.f / (d.ftrl_alpha * w[W_MX] * (w[W_G2] + w[W_MX]));
  w[W_XT] = std::sqrt(w[W_G2]) * d.ftrl_beta * w[W_ZT] * correctedExp(squared_theta / 2.f * tmp) * tmp;

  d.predict += w[W_XT] * x;
}

void inner_update_pistol_post(update_data& d, float x, float& wref)
{
  float* w = &wref;
  float gradient = d.update * x;

  w[W_ZT] += -gradient;
  w[W_G2] += fabs(gradient);
}

// Coin betting vectors
// W_XT 0  current parameter
// W_ZT 1  sum negative gradients
// W_G2 2  sum of absolute value of gradients
// W_MX 3  maximum absolute value
// W_WE 4  Wealth
// W_MG 5  Maximum Lipschitz constant
void inner_update_cb_state_and_predict(update_data& d, float x, float& wref)
{
  float* w = &wref;
  float w_mx = w[W_MX];
  float w_xt = 0.0;

  float fabs_x = fabs(x);
  if (fabs_x > w_mx)
  {
    w_mx = fabs_x;
  }

  // COCOB update without sigmoid
  if (w[W_MG] * w_mx > 0)
    w_xt = (d.ftrl_alpha + w[W_WE]) * w[W_ZT] / (w[W_MG] * w_mx * (w[W_MG] * w_mx + w[W_G2]));

  d.predict += w_xt * x;
  if (w_mx > 0)
    d.normalized_squared_norm_x += x * x / (w_mx * w_mx);
}

void inner_update_cb_post(update_data& d, float x, float& wref)
{
  float* w = &wref;
  float fabs_x = fabs(x);
  float gradient = d.update * x;

  if (fabs_x > w[W_MX])
  {
    w[W_MX] = fabs_x;
  }

  float fabs_gradient = fabs(d.update);
  if (fabs_gradient > w[W_MG])
    w[W_MG] = fabs_gradient > d.ftrl_beta ? fabs_gradient : d.ftrl_beta;

  // COCOB update without sigmoid.
  // If a new Lipschitz constant and/or magnitude of x is found, the w is
  // recalculated and used in the update of the wealth below.
  if (w[W_MG] * w[W_MX] > 0)
    w[W_XT] = (d.ftrl_alpha + w[W_WE]) * w[W_ZT] / (w[W_MG] * w[W_MX] * (w[W_MG] * w[W_MX] + w[W_G2]));
  else
    w[W_XT] = 0;

  w[W_ZT] += -gradient;
  w[W_G2] += fabs(gradient);
  w[W_WE] += (-gradient * w[W_XT]);
}

void update_state_and_predict_cb(ftrl& b, single_learner&, example& ec)
{
  b.data.predict = 0;
  b.data.normalized_squared_norm_x = 0;

  GD::foreach_feature<update_data, inner_update_cb_state_and_predict>(*b.all, ec, b.data);

  b.all->normalized_sum_norm_x += ((double)ec.weight) * b.data.normalized_squared_norm_x;
  b.total_weight += ec.weight;

  ec.partial_prediction = b.data.predict / ((float)((b.all->normalized_sum_norm_x + 1e-6) / b.total_weight));

  ec.pred.scalar = GD::finalize_prediction(b.all->sd, ec.partial_prediction);
}

void update_state_and_predict_pistol(ftrl& b, single_learner&, example& ec)
{
  b.data.predict = 0;

  GD::foreach_feature<update_data, inner_update_pistol_state_and_predict>(*b.all, ec, b.data);
  ec.partial_prediction = b.data.predict;
  ec.pred.scalar = GD::finalize_prediction(b.all->sd, ec.partial_prediction);
}

void update_after_prediction_proximal(ftrl& b, example& ec)
{
  b.data.update = b.all->loss->first_derivative(b.all->sd, ec.pred.scalar, ec.l.simple.label) * ec.weight;

  GD::foreach_feature<update_data, inner_update_proximal>(*b.all, ec, b.data);
}

void update_after_prediction_pistol(ftrl& b, example& ec)
{
  b.data.update = b.all->loss->first_derivative(b.all->sd, ec.pred.scalar, ec.l.simple.label) * ec.weight;

  GD::foreach_feature<update_data, inner_update_pistol_post>(*b.all, ec, b.data);
}

void update_after_prediction_cb(ftrl& b, example& ec)
{
  b.data.update = b.all->loss->first_derivative(b.all->sd, ec.pred.scalar, ec.l.simple.label) * ec.weight;

  GD::foreach_feature<update_data, inner_update_cb_post>(*b.all, ec, b.data);
}

template <bool audit>
void learn_proximal(ftrl& a, single_learner& base, example& ec)
{
  assert(ec.in_use);

  // predict with confidence
  predict<audit>(a, base, ec);

  // update state based on the prediction
  update_after_prediction_proximal(a, ec);
}

void learn_pistol(ftrl& a, single_learner& base, example& ec)
{
  assert(ec.in_use);

  // update state based on the example and predict
  update_state_and_predict_pistol(a, base, ec);

  // update state based on the prediction
  update_after_prediction_pistol(a, ec);
}

void learn_cb(ftrl& a, single_learner& base, example& ec)
{
  assert(ec.in_use);

  // update state based on the example and predict
  update_state_and_predict_cb(a, base, ec);

  // update state based on the prediction
  update_after_prediction_cb(a, ec);
}

void save_load(ftrl& b, io_buf& model_file, bool read, bool text)
{
  vw* all = b.all;
  if (read)
    initialize_regressor(*all);

  if (!model_file.files.empty())
  {
    bool resume = all->save_resume;
    std::stringstream msg;
    msg << ":" << resume << "\n";
    bin_text_read_write_fixed(model_file, (char*)&resume, sizeof(resume), "", read, msg, text);

    if (resume)
      GD::save_load_online_state(*all, model_file, read, text, b.total_weight, nullptr, b.ftrl_size);
    else
      GD::save_load_regressor(*all, model_file, read, text);
  }
}

void end_pass(ftrl& g)
{
  vw& all = *g.all;

  if (!all.holdout_set_off)
  {
    if (summarize_holdout_set(all, g.no_win_counter))
      finalize_regressor(all, all.final_regressor_name);
    if ((g.early_stop_thres == g.no_win_counter) &&
        ((all.check_holdout_every_n_passes <= 1) || ((all.current_pass % all.check_holdout_every_n_passes) == 0)))
      set_done(all);
  }
}

base_learner* ftrl_setup(options_i& options, vw& all)
{
  auto b = scoped_calloc_or_throw<ftrl>();
  bool ftrl_option = false;
  bool pistol = false;
  bool coin = false;

  option_group_definition new_options("Follow the Regularized Leader");
  new_options.add(make_option("ftrl", ftrl_option).keep().help("FTRL: Follow the Proximal Regularized Leader"))
      .add(make_option("coin", coin).keep().help("Coin betting optimizer"))
      .add(make_option("pistol", pistol).keep().help("PiSTOL: Parameter-free STOchastic Learning"))
      .add(make_option("ftrl_alpha", b->ftrl_alpha).help("Learning rate for FTRL optimization"))
      .add(make_option("ftrl_beta", b->ftrl_beta).help("Learning rate for FTRL optimization"));
  options.add_and_parse(new_options);

  if (!ftrl_option && !pistol && !coin)
  {
    return nullptr;
  }

  // Defaults that are specific to the mode that was chosen.
  if (ftrl_option)
  {
    b->ftrl_alpha = options.was_supplied("ftrl_alpha") ? b->ftrl_alpha : 0.005f;
    b->ftrl_beta = options.was_supplied("ftrl_beta") ? b->ftrl_beta : 0.1f;
  }
  else if (pistol)
  {
    b->ftrl_alpha = options.was_supplied("ftrl_alpha") ? b->ftrl_alpha : 1.0f;
    b->ftrl_beta = options.was_supplied("ftrl_beta") ? b->ftrl_beta : 0.5f;
  }
  else if (coin)
  {
    b->ftrl_alpha = options.was_supplied("ftrl_alpha") ? b->ftrl_alpha : 4.0f;
    b->ftrl_beta = options.was_supplied("ftrl_beta") ? b->ftrl_beta : 1.0f;
  }

  b->all = &all;
  b->no_win_counter = 0;
  b->all->normalized_sum_norm_x = 0;
  b->total_weight = 0;

  void (*learn_ptr)(ftrl&, single_learner&, example&) = nullptr;

  std::string algorithm_name;
  if (ftrl_option)
  {
    algorithm_name = "Proximal-FTRL";
    if (all.audit)
      learn_ptr = learn_proximal<true>;
    else
      learn_ptr = learn_proximal<false>;
    all.weights.stride_shift(2);  // NOTE: for more parameter storage
    b->ftrl_size = 3;
  }
  else if (pistol)
  {
    algorithm_name = "PiSTOL";
    learn_ptr = learn_pistol;
    all.weights.stride_shift(2);  // NOTE: for more parameter storage
    b->ftrl_size = 4;
  }
  else if (coin)
  {
    algorithm_name = "Coin Betting";
    learn_ptr = learn_cb;
    all.weights.stride_shift(3);  // NOTE: for more parameter storage
    b->ftrl_size = 6;
  }

  b->data.ftrl_alpha = b->ftrl_alpha;
  b->data.ftrl_beta = b->ftrl_beta;
  b->data.l1_lambda = b->all->l1_lambda;
  b->data.l2_lambda = b->all->l2_lambda;

  if (!all.quiet)
  {
    std::cerr << "Enabling FTRL based optimization" << std::endl;
    std::cerr << "Algorithm used: " << algorithm_name << std::endl;
    std::cerr << "ftrl_alpha = " << b->ftrl_alpha << std::endl;
    std::cerr << "ftrl_beta = " << b->ftrl_beta << std::endl;
  }

  if (!all.holdout_set_off)
  {
    all.sd->holdout_best_loss = FLT_MAX;
    b->early_stop_thres = options.get_typed_option<size_t>("early_terminate").value();
  }

  learner<ftrl, example>* l;
  if (all.audit || all.hash_inv)
    l = &init_learner(b, learn_ptr, predict<true>, UINT64_ONE << all.weights.stride_shift());
  else
    l = &init_learner(b, learn_ptr, predict<false>, UINT64_ONE << all.weights.stride_shift());
  l->set_sensitivity(sensitivity);
  if (all.audit || all.hash_inv)
    l->set_multipredict(multipredict<true>);
  else
    l->set_multipredict(multipredict<false>);
  l->set_save_load(save_load);
  l->set_end_pass(end_pass);
  return make_base(*l);
}
