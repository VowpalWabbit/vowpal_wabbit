// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/freegrad.h"

#include "vw/core/crossplat_compat.h"
#include "vw/core/global_data.h"
#include "vw/core/learner.h"
#include "vw/core/loss_functions.h"
#include "vw/core/memory.h"
#include "vw/core/parse_regressor.h"
#include "vw/core/parser.h"
#include "vw/core/reductions/gd.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/simple_label.h"

#include <cfloat>
#include <cmath>
#include <string>

using namespace VW::LEARNER;
using namespace VW::config;

namespace
{
constexpr size_t W = 0;      // current parameter
constexpr size_t G_SUM = 1;  // sum of gradients
constexpr size_t V_SUM = 2;  // sum of squared gradients
constexpr size_t H1 = 3;     // maximum absolute value of features
constexpr size_t HT = 4;     // maximum gradient
constexpr size_t S = 5;      // sum of radios \sum_s |x_s|/h_s

class freegrad;
class freegrad_update_data
{
public:
  freegrad* freegrad_data_ptr;
  float update = 0.f;
  float ec_weight = 0.f;
  float predict = 0.f;
  float squared_norm_prediction = 0.f;
  float grad_dot_w = 0.f;
  float squared_norm_clipped_grad = 0.f;
  float sum_normalized_grad_norms = 0.f;
  float maximum_clipped_gradient_norm = 0.f;
};

class freegrad
{
public:
  VW::workspace* all;  // features, finalize, l1, l2,
  float epsilon = 0.f;
  float lipschitz_const = 0.f;
  bool restart;
  bool project;
  bool adaptiveradius;
  float radius = 0.f;
  freegrad_update_data update_data;
  size_t no_win_counter;
  size_t early_stop_thres;
  uint32_t freegrad_size;
  double total_weight = 0.0;
  double normalized_sum_norm_x = 0.0;
};

template <bool audit>
void predict(freegrad& b, VW::example& ec)
{
  size_t num_features_from_interactions = 0;
  ec.partial_prediction = VW::inline_predict(*b.all, ec, num_features_from_interactions);
  ec.num_features_from_interactions = num_features_from_interactions;
  ec.pred.scalar = VW::details::finalize_prediction(*b.all->sd, b.all->logger, ec.partial_prediction);
  if (audit) { VW::details::print_audit_features(*(b.all), ec); }
}

void inner_freegrad_predict(freegrad_update_data& d, float x, float& wref)
{
  float* w = &wref;
  float h1 = w[H1];           // will be set to the value of the first non-zero gradient w.r.t. the scalar feature x
  float ht = w[HT];           // maximum absolute value of the gradient w.r.t. scalar feature x
  float w_pred = 0.0;         // weight for the feature x
  float G = w[G_SUM];         // NOLINT sum of gradients w.r.t. scalar feature x
  float absG = std::fabs(G);  // NOLINT
  float V = w[V_SUM];         // NOLINT sum of squared gradients w.r.t. scalar feature x
  float epsilon = d.freegrad_data_ptr->epsilon;

  // Only predict a non-zero w_pred if a non-zero gradient has been observed
  // freegrad update Equation 9 in paper http://proceedings.mlr.press/v125/mhammedi20a/mhammedi20a.pdf
  if (h1 > 0)
  {
    w_pred = -G * epsilon * (2.f * V + ht * absG) * std::pow(h1, 2.f) /
        (2.f * std::pow(V + ht * absG, 2.f) * sqrtf(V)) * std::exp(std::pow(absG, 2.f) / (2.f * V + 2.f * ht * absG));
  }

  d.squared_norm_prediction += std::pow(w_pred, 2.f);
  // This is the unprojected predict
  d.predict += w_pred * x;
}

void freegrad_predict(freegrad& fg, VW::example& ec)
{
  fg.update_data.predict = 0.;
  fg.update_data.squared_norm_prediction = 0.;
  size_t num_features_from_interactions = 0;
  fg.total_weight += ec.weight;
  float norm_w_pred;
  float projection_radius;

  // Compute the unprojected predict
  VW::foreach_feature<freegrad_update_data, inner_freegrad_predict>(
      *fg.all, ec, fg.update_data, num_features_from_interactions);
  norm_w_pred = sqrtf(fg.update_data.squared_norm_prediction);

  if (fg.project)
  {
    // Set the project radius either to the user-specified value, or adap tively
    if (fg.adaptiveradius) { projection_radius = fg.epsilon * sqrtf(fg.update_data.sum_normalized_grad_norms); }
    else { projection_radius = fg.radius; }
    // Compute the projected predict if applicable
    if (norm_w_pred > projection_radius) { fg.update_data.predict *= projection_radius / norm_w_pred; }
  }
  ec.partial_prediction = fg.update_data.predict;

  ec.num_features_from_interactions = num_features_from_interactions;
  ec.pred.scalar = VW::details::finalize_prediction(*fg.all->sd, fg.all->logger, ec.partial_prediction);
}

void gradient_dot_w(freegrad_update_data& d, float x, float& wref)
{
  float* w = &wref;
  float h1 = w[H1];           // will be set to the value of the first non-zero gradient w.r.t. the scalar feature x
  float ht = w[HT];           // maximum absolute value of the gradient w.r.t. scalar feature x
  float w_pred = 0.0;         // weight for the feature x
  float G = w[G_SUM];         // NOLINT sum of gradients w.r.t. scalar feature x
  float absG = std::fabs(G);  // NOLINT
  float V = w[V_SUM];         // NOLINT sum of squared gradients w.r.t. scalar feature x
  float epsilon = d.freegrad_data_ptr->epsilon;
  float gradient = d.update * x;

  // Only predict a non-zero w_pred if a non-zero gradient has been observed
  if (h1 > 0.f)
  {
    w_pred = -G * epsilon * (2.f * V + ht * absG) * std::pow(h1, 2.f) /
        (2.f * std::pow(V + ht * absG, 2.f) * sqrtf(V)) * std::exp(std::pow(absG, 2.f) / (2 * V + 2.f * ht * absG));
  }

  d.grad_dot_w += gradient * w_pred;
}

void inner_freegrad_update_after_prediction(freegrad_update_data& d, float x, float& wref)
{
  float* w = &wref;
  float gradient = d.update * x;
  float tilde_gradient = gradient;
  float clipped_gradient;
  float fabs_g = std::fabs(gradient);
  _UNUSED(fabs_g);
  float g_dot_w = d.grad_dot_w;
  float norm_w_pred = sqrtf(d.squared_norm_prediction);
  float projection_radius;
  float fabs_tilde_g;

  float h1 = w[H1];    // will be set to the value of the first non-zero gradient w.r.t. the scalar feature x
  float ht = w[HT];    // maximum absolute value of the gradient w.r.t. scalar feature x
  float w_pred = 0.0;  // weight for the feature x
  _UNUSED(w_pred);
  float G = w[G_SUM];         // NOLINT sum of gradients w.r.t. scalar feature x
  float absG = std::fabs(G);  // NOLINT
  float V = w[V_SUM];         // NOLINT sum of squared gradients w.r.t. scalar feature x
  float epsilon = d.freegrad_data_ptr->epsilon;
  float lipschitz_const = d.freegrad_data_ptr->lipschitz_const;

  // Computing the freegrad prediction again (Eq.(9) and Line 7 of Alg. 2 in paper)
  if (h1 > 0)
  {
    w[W] = -G * epsilon * (2.f * V + ht * absG) * std::pow(h1, 2.f) / (2.f * std::pow(V + ht * absG, 2.f) * sqrtf(V)) *
        std::exp(std::pow(absG, 2.f) / (2 * V + 2.f * ht * absG));
  }

  // Compute the tilted gradient:
  // Cutkosky's varying constrains' reduction in
  // Alg. 1 in http://proceedings.mlr.press/v119/cutkosky20a/cutkosky20a.pdf with sphere sets
  if (d.freegrad_data_ptr->project)
  {
    // Set the project radius either to the user-specified value, or adaptively
    if (d.freegrad_data_ptr->adaptiveradius)
    {
      projection_radius = d.freegrad_data_ptr->epsilon * sqrtf(d.sum_normalized_grad_norms);
    }
    else { projection_radius = d.freegrad_data_ptr->radius; }

    if (norm_w_pred > projection_radius && g_dot_w < 0)
    {
      tilde_gradient = gradient - (g_dot_w * w[W]) / std::pow(norm_w_pred, 2.f);
    }
  }

  // Only do something if a non-zero gradient has been observed
  if (tilde_gradient == 0) { return; }

  clipped_gradient = tilde_gradient;
  fabs_tilde_g = std::fabs(tilde_gradient);

  // Updating the hint sequence
  if (h1 == 0 && lipschitz_const == 0)
  {
    w[H1] = fabs_tilde_g;
    w[HT] = fabs_tilde_g;
    w[V_SUM] += d.ec_weight * std::pow(fabs_tilde_g, 2.f);
  }
  else if (h1 == 0)
  {
    w[H1] = lipschitz_const;
    w[HT] = lipschitz_const;
    w[V_SUM] += d.ec_weight * std::pow(fabs_tilde_g, 2.f);
  }
  else if (fabs_tilde_g > ht)
  {
    // Perform gradient clipping if necessary
    clipped_gradient *= ht / fabs_tilde_g;
    w[HT] = fabs_tilde_g;
  }
  d.squared_norm_clipped_grad += std::pow(clipped_gradient, 2.f);

  // Check if restarts are enabled and whether the condition is satisfied
  if (d.freegrad_data_ptr->restart && w[HT] / w[H1] > w[S] + 2)
  {
    // Do a restart, but keep the lastest hint info
    w[H1] = w[HT];
    w[G_SUM] = clipped_gradient + (d.ec_weight - 1) * tilde_gradient;
    w[V_SUM] = std::pow(clipped_gradient, 2.f) + (d.ec_weight - 1) * std::pow(tilde_gradient, 2.f);
  }
  else
  {
    // Updating the gradient information
    w[G_SUM] += clipped_gradient + (d.ec_weight - 1) * tilde_gradient;
    w[V_SUM] += std::pow(clipped_gradient, 2.f) + (d.ec_weight - 1) * std::pow(tilde_gradient, 2.f);
  }
  if (ht > 0) { w[S] += std::fabs(clipped_gradient) / ht + (d.ec_weight - 1) * std::fabs(tilde_gradient) / w[HT]; }
}

void freegrad_update_after_prediction(freegrad& fg, VW::example& ec)
{
  float clipped_grad_norm;
  fg.update_data.grad_dot_w = 0.;
  fg.update_data.squared_norm_clipped_grad = 0.;
  fg.update_data.ec_weight = (float)ec.weight;

  // Partial derivative of loss (Note that the weight of the examples ec is not accounted for at this stage. This is
  // done in inner_freegrad_update_after_prediction)
  fg.update_data.update = fg.all->loss->first_derivative(fg.all->sd.get(), ec.pred.scalar, ec.l.simple.label);

  // Compute gradient norm
  VW::foreach_feature<freegrad_update_data, gradient_dot_w>(*fg.all, ec, fg.update_data);

  // Performing the update
  VW::foreach_feature<freegrad_update_data, inner_freegrad_update_after_prediction>(*fg.all, ec, fg.update_data);

  // Update the maximum gradient norm value
  clipped_grad_norm = sqrtf(fg.update_data.squared_norm_clipped_grad);
  if (clipped_grad_norm > fg.update_data.maximum_clipped_gradient_norm)
  {
    fg.update_data.maximum_clipped_gradient_norm = clipped_grad_norm;
  }

  if (fg.update_data.maximum_clipped_gradient_norm > 0)
  {
    fg.update_data.sum_normalized_grad_norms +=
        fg.update_data.ec_weight * clipped_grad_norm / fg.update_data.maximum_clipped_gradient_norm;
  }
}

template <bool audit>
void learn_freegrad(freegrad& a, VW::example& ec)
{
  // update state based on the example and predict
  freegrad_predict(a, ec);
  if (audit) { VW::details::print_audit_features(*(a.all), ec); }

  // update state based on the prediction
  freegrad_update_after_prediction(a, ec);
}

void save_load(freegrad& fg, VW::io_buf& model_file, bool read, bool text)
{
  VW::workspace* all = fg.all;
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
          *all, model_file, read, text, fg.total_weight, fg.normalized_sum_norm_x, nullptr, fg.freegrad_size);
    }
    else { VW::details::save_load_regressor_gd(*all, model_file, read, text); }
  }
}

void end_pass(freegrad& fg)
{
  VW::workspace& all = *fg.all;

  if (!all.holdout_set_off)
  {
    if (VW::details::summarize_holdout_set(all, fg.no_win_counter))
    {
      VW::details::finalize_regressor(all, all.final_regressor_name);
    }
    if ((fg.early_stop_thres == fg.no_win_counter) &&
        ((all.check_holdout_every_n_passes <= 1) || ((all.current_pass % all.check_holdout_every_n_passes) == 0)))
    {
      VW::details::set_done(all);
    }
  }
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::freegrad_setup(VW::setup_base_i& stack_builder)
{
  auto& options = *stack_builder.get_options();
  bool freegrad_enabled;
  bool restart = false;
  bool project = false;
  bool adaptiveradius = true;
  float radius;
  float fepsilon;
  float flipschitz_const;

  option_group_definition new_options("[Reduction] FreeGrad");
  new_options.add(make_option("freegrad", freegrad_enabled).necessary().keep().help("Diagonal FreeGrad Algorithm"))
      .add(make_option("restart", restart).help("Use the FreeRange restarts"))
      .add(make_option("project", project)
               .help("Project the outputs to adapt to both the lipschitz and comparator norm"))
      .add(make_option("radius", radius)
               .help("Radius of the l2-ball for the projection. If not supplied, an adaptive radius will be used"))
      .add(make_option("fepsilon", fepsilon).default_value(1.f).help("Initial wealth"))
      .add(make_option("flipschitz_const", flipschitz_const)
               .default_value(0.f)
               .help("Upper bound on the norm of the gradients if known in advance"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  auto fg_ptr = VW::make_unique<freegrad>();
  if (options.was_supplied("radius"))
  {
    fg_ptr->radius = radius;
    adaptiveradius = false;
  }

  // Defaults
  fg_ptr->update_data.sum_normalized_grad_norms = 1;
  fg_ptr->update_data.maximum_clipped_gradient_norm = 0.;
  fg_ptr->update_data.freegrad_data_ptr = fg_ptr.get();

  fg_ptr->all = stack_builder.get_all_pointer();
  fg_ptr->restart = restart;
  fg_ptr->project = project;
  fg_ptr->adaptiveradius = adaptiveradius;
  fg_ptr->no_win_counter = 0;
  fg_ptr->total_weight = 0;
  fg_ptr->normalized_sum_norm_x = 0;
  fg_ptr->epsilon = fepsilon;
  fg_ptr->lipschitz_const = flipschitz_const;

  const auto* algorithm_name = "FreeGrad";

  fg_ptr->all->weights.stride_shift(3);  // NOTE: for more parameter storage
  fg_ptr->freegrad_size = 6;

  if (!fg_ptr->all->quiet)
  {
    *(fg_ptr->all->trace_message) << "Enabling FreeGrad based optimization" << std::endl;
    *(fg_ptr->all->trace_message) << "Algorithm used: " << algorithm_name << std::endl;
  }

  if (!fg_ptr->all->holdout_set_off)
  {
    fg_ptr->all->sd->holdout_best_loss = FLT_MAX;
    fg_ptr->early_stop_thres = options.get_typed_option<uint64_t>("early_terminate").value();
  }

  auto predict_ptr = (fg_ptr->all->audit || fg_ptr->all->hash_inv) ? predict<true> : predict<false>;
  auto learn_ptr = (fg_ptr->all->audit || fg_ptr->all->hash_inv) ? learn_freegrad<true> : learn_freegrad<false>;
  auto l =
      VW::LEARNER::make_bottom_learner(std::move(fg_ptr), learn_ptr, predict_ptr,
          stack_builder.get_setupfn_name(freegrad_setup), VW::prediction_type_t::SCALAR, VW::label_type_t::SIMPLE)
          .set_learn_returns_prediction(true)
          .set_params_per_weight(VW::details::UINT64_ONE << stack_builder.get_all_pointer()->weights.stride_shift())
          .set_save_load(save_load)
          .set_end_pass(end_pass)
          .set_output_example_prediction(VW::details::output_example_prediction_simple_label<freegrad>)
          .set_update_stats(VW::details::update_stats_simple_label<freegrad>)
          .set_print_update(VW::details::print_update_simple_label<freegrad>)
          .build();

  return l;
}