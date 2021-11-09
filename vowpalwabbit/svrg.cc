
// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cassert>
#include <iostream>

#include "gd.h"
#include "memory.h"
#include "reductions_fwd.h"
#include "vw.h"
#include "reductions.h"

using namespace VW::LEARNER;
using namespace VW::config;

namespace SVRG
{
#define W_INNER 0       // working "inner-loop" weights, updated per example
#define W_STABLE 1      // stable weights, updated per stage
#define W_STABLEGRAD 2  // gradient corresponding to stable weights

struct svrg
{
  int stage_size = 1;         // Number of data passes per stage.
  int prev_pass = -1;         // To detect that we're in a new pass.
  int stable_grad_count = 0;  // Number of data points that
  // contributed to the stable gradient
  // calculation.

  // The VW process' global state.
  VW::workspace* all = nullptr;

  svrg(VW::workspace* all) : all(all) {}
};

// Mimic GD::inline_predict but with offset for predicting with either
// stable versus inner weights.

template <int offset>
inline void vec_add(float& p, const float x, float& w)
{
  float* ws = &w;
  p += x * ws[offset];
}

template <int offset>
inline float inline_predict(VW::workspace& all, example& ec)
{
  const auto& simple_red_features = ec._reduction_features.template get<simple_label_reduction_features>();
  float acc = simple_red_features.initial;
  GD::foreach_feature<float, vec_add<offset> >(all, ec, acc);
  return acc;
}

// -- Prediction, using inner vs. stable weights --

float predict_stable(const svrg& s, example& ec)
{
  return GD::finalize_prediction(s.all->sd, s.all->logger, inline_predict<W_STABLE>(*s.all, ec));
}

void predict(svrg& s, base_learner&, example& ec)
{
  ec.partial_prediction = inline_predict<W_INNER>(*s.all, ec);
  ec.pred.scalar = GD::finalize_prediction(s.all->sd, s.all->logger, ec.partial_prediction);
}

float gradient_scalar(const svrg& s, const example& ec, float pred)
{
  return s.all->loss->first_derivative(s.all->sd, pred, ec.l.simple.label) * ec.weight;
}

// -- Updates, taking inner steps vs. accumulating a full gradient --

struct update
{
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

void update_inner(const svrg& s, example& ec)
{
  update u;
  // |ec| already has prediction according to inner weights.
  u.g_scalar_inner = gradient_scalar(s, ec, ec.pred.scalar);
  u.g_scalar_stable = gradient_scalar(s, ec, predict_stable(s, ec));
  u.eta = s.all->eta;
  u.norm = static_cast<float>(s.stable_grad_count);
  GD::foreach_feature<update, update_inner_feature>(*s.all, ec, u);
}

void update_stable(const svrg& s, example& ec)
{
  float g = gradient_scalar(s, ec, predict_stable(s, ec));
  GD::foreach_feature<float, update_stable_feature>(*s.all, ec, g);
}

void learn(svrg& s, base_learner& base, example& ec)
{
  predict(s, base, ec);

  const int pass = static_cast<int>(s.all->passes_complete);

  if (pass % (s.stage_size + 1) == 0)  // Compute exact gradient
  {
    if (s.prev_pass != pass && !s.all->logger.quiet)
    {
      *(s.all->trace_message) << "svrg pass " << pass << ": committing stable point" << std::endl;
      for (uint32_t j = 0; j < VW::num_weights(*s.all); j++)
      {
        float w = VW::get_weight(*s.all, j, W_INNER);
        VW::set_weight(*s.all, j, W_STABLE, w);
        VW::set_weight(*s.all, j, W_STABLEGRAD, 0.f);
      }
      s.stable_grad_count = 0;
      *(s.all->trace_message) << "svrg pass " << pass << ": computing exact gradient" << std::endl;
    }
    update_stable(s, ec);
    s.stable_grad_count++;
  }
  else  // Perform updates
  {
    if (s.prev_pass != pass && !s.all->logger.quiet)
    { *(s.all->trace_message) << "svrg pass " << pass << ": taking steps" << std::endl; }
    update_inner(s, ec);
  }

  s.prev_pass = pass;
}

void save_load(svrg& s, io_buf& model_file, bool read, bool text)
{
  if (read) { initialize_regressor(*s.all); }

  if (model_file.num_files() != 0)
  {
    bool resume = s.all->save_resume;
    std::stringstream msg;
    msg << ":" << resume << "\n";
    bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&resume), sizeof(resume), read, msg, text);

    double temp = 0.;
    if (resume)
      GD::save_load_online_state(*s.all, model_file, read, text, temp);
    else
      GD::save_load_regressor(*s.all, model_file, read, text);
  }
}

}  // namespace SVRG

using namespace SVRG;

base_learner* svrg_setup(VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto s = VW::make_unique<svrg>(&all);

  bool svrg_option = false;
  option_group_definition new_options("Stochastic Variance Reduced Gradient");
  new_options
      .add(make_option("svrg", svrg_option).keep().necessary().help("Streaming Stochastic Variance Reduced Gradient"))
      .add(make_option("stage_size", s->stage_size).default_value(1).help("Number of passes per SVRG stage"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  // Request more parameter storage (4 floats per feature)
  all.weights.stride_shift(2);
  auto* l = VW::LEARNER::make_base_learner(std::move(s), learn, predict, stack_builder.get_setupfn_name(svrg_setup),
      VW::prediction_type_t::scalar, VW::label_type_t::simple)
                .set_params_per_weight(UINT64_ONE << all.weights.stride_shift())
                .set_save_load(save_load)
                .build();
  return make_base(*l);
}
