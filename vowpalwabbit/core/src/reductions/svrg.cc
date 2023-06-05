
// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/svrg.h"

#include "vw/core/crossplat_compat.h"
#include "vw/core/learner.h"
#include "vw/core/loss_functions.h"
#include "vw/core/memory.h"
#include "vw/core/parse_regressor.h"
#include "vw/core/prediction_type.h"
#include "vw/core/reductions/gd.h"
#include "vw/core/setup_base.h"
#include "vw/core/vw.h"
#include "vw/core/vw_fwd.h"

#include <cassert>
#include <iostream>

using namespace VW::LEARNER;
using namespace VW::config;

namespace
{
#define W_INNER 0       // working "inner-loop" weights, updated per example
#define W_STABLE 1      // stable weights, updated per stage
#define W_STABLEGRAD 2  // gradient corresponding to stable weights

class svrg
{
public:
  int stage_size = 1;         // Number of data passes per stage.
  int prev_pass = -1;         // To detect that we're in a new pass.
  int stable_grad_count = 0;  // Number of data points that
  // contributed to the stable gradient
  // calculation.

  // The VW process' global state.
  VW::workspace* all = nullptr;

  svrg(VW::workspace* all) : all(all) {}
};

// Mimic VW::inline_predict but with offset for predicting with either
// stable versus inner weights.

template <int offset>
inline void vec_add(float& p, const float x, float& w)
{
  float* ws = &w;
  p += x * ws[offset];
}

template <int offset>
inline float inline_predict(VW::workspace& all, VW::example& ec)
{
  const auto& simple_red_features = ec.ex_reduction_features.template get<VW::simple_label_reduction_features>();
  float acc = simple_red_features.initial;
  VW::foreach_feature<float, vec_add<offset> >(all, ec, acc);
  return acc;
}

// -- Prediction, using inner vs. stable weights --

float predict_stable(const svrg& s, VW::example& ec)
{
  return VW::details::finalize_prediction(*s.all->sd, s.all->logger, inline_predict<W_STABLE>(*s.all, ec));
}

void predict(svrg& s, VW::example& ec)
{
  ec.partial_prediction = inline_predict<W_INNER>(*s.all, ec);
  ec.pred.scalar = VW::details::finalize_prediction(*s.all->sd, s.all->logger, ec.partial_prediction);
}

float gradient_scalar(const svrg& s, const VW::example& ec, float pred)
{
  return s.all->loss_config.loss->first_derivative(s.all->sd.get(), pred, ec.l.simple.label) * ec.weight;
}

// -- Updates, taking inner steps vs. accumulating a full gradient --

class update
{
public:
  float g_scalar_stable;
  float g_scalar_inner;
  float eta;
  float norm;
};

inline void update_inner_feature(update& u, float x, float& w)
{
  float* ws = &w;
  w -= u.eta * ((u.g_scalar_inner - u.g_scalar_stable) * x + ws[W_STABLEGRAD] / u.norm);
}

inline void update_stable_feature(float& g_scalar, float x, float& w)
{
  float* ws = &w;
  ws[W_STABLEGRAD] += g_scalar * x;
}

void update_inner(const svrg& s, VW::example& ec)
{
  update u;
  // |ec| already has prediction according to inner weights.
  u.g_scalar_inner = gradient_scalar(s, ec, ec.pred.scalar);
  u.g_scalar_stable = gradient_scalar(s, ec, predict_stable(s, ec));
  u.eta = s.all->update_rule_config.eta;
  u.norm = static_cast<float>(s.stable_grad_count);
  VW::foreach_feature<update, update_inner_feature>(*s.all, ec, u);
}

void update_stable(const svrg& s, VW::example& ec)
{
  float g = gradient_scalar(s, ec, predict_stable(s, ec));
  VW::foreach_feature<float, update_stable_feature>(*s.all, ec, g);
}

void learn(svrg& s, VW::example& ec)
{
  predict(s, ec);

  const int pass = static_cast<int>(s.all->runtime_state.passes_complete);

  if (pass % (s.stage_size + 1) == 0)  // Compute exact gradient
  {
    if (s.prev_pass != pass && !s.all->output_config.quiet)
    {
      *(s.all->output_runtime.trace_message) << "svrg pass " << pass << ": committing stable point" << std::endl;
      for (uint32_t j = 0; j < VW::num_weights(*s.all); j++)
      {
        float w = VW::get_weight(*s.all, j, W_INNER);
        VW::set_weight(*s.all, j, W_STABLE, w);
        VW::set_weight(*s.all, j, W_STABLEGRAD, 0.f);
      }
      s.stable_grad_count = 0;
      *(s.all->output_runtime.trace_message) << "svrg pass " << pass << ": computing exact gradient" << std::endl;
    }
    update_stable(s, ec);
    s.stable_grad_count++;
  }
  else  // Perform updates
  {
    if (s.prev_pass != pass && !s.all->output_config.quiet)
    {
      *(s.all->output_runtime.trace_message) << "svrg pass " << pass << ": taking steps" << std::endl;
    }
    update_inner(s, ec);
  }

  s.prev_pass = pass;
}

void save_load(svrg& s, VW::io_buf& model_file, bool read, bool text)
{
  if (read) { VW::details::initialize_regressor(*s.all); }

  if (model_file.num_files() != 0)
  {
    bool resume = s.all->output_model_config.save_resume;
    std::stringstream msg;
    msg << ":" << resume << "\n";
    VW::details::bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&resume), sizeof(resume), read, msg, text);

    std::vector<VW::reductions::details::gd_per_model_state> temp_pms = {VW::reductions::details::gd_per_model_state()};
    if (resume) { VW::details::save_load_online_state_gd(*s.all, model_file, read, text, temp_pms); }
    else { VW::details::save_load_regressor_gd(*s.all, model_file, read, text); }
  }
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::svrg_setup(VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto s = VW::make_unique<svrg>(&all);

  bool svrg_option = false;
  option_group_definition new_options("[Reduction] Stochastic Variance Reduced Gradient");
  new_options
      .add(make_option("svrg", svrg_option).keep().necessary().help("Streaming Stochastic Variance Reduced Gradient"))
      .add(make_option("stage_size", s->stage_size).default_value(1).help("Number of passes per SVRG stage"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  // Request more parameter storage (4 floats per feature)
  all.weights.stride_shift(2);
  auto l = make_bottom_learner(std::move(s), learn, predict, stack_builder.get_setupfn_name(svrg_setup),
      VW::prediction_type_t::SCALAR, VW::label_type_t::SIMPLE)
               .set_output_example_prediction(VW::details::output_example_prediction_simple_label<svrg>)
               .set_update_stats(VW::details::update_stats_simple_label<svrg>)
               .set_print_update(VW::details::print_update_simple_label<svrg>)
               .set_save_load(save_load)
               .build();
  return l;
}
