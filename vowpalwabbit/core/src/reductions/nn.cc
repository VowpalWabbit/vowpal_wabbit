// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/nn.h"

#include "vw/common/random.h"
#include "vw/config/options.h"
#include "vw/core/constant.h"
#include "vw/core/guard.h"
#include "vw/core/learner.h"
#include "vw/core/loss_functions.h"
#include "vw/core/named_labels.h"
#include "vw/core/reductions/gd.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <memory>
#include <sstream>

using namespace VW::LEARNER;
using namespace VW::config;

namespace
{
constexpr float HIDDEN_MIN_ACTIVATION = -3;
constexpr float HIDDEN_MAX_ACTIVATION = 3;
constexpr uint64_t NN_CONSTANT = 533357803;

class nn
{
public:
  uint32_t k = 0;
  std::unique_ptr<VW::loss_function> squared_loss;
  VW::example output_layer;
  VW::example hiddenbias;
  VW::example outputweight;
  float prediction = 0.f;
  size_t increment = 0;
  bool dropout = false;
  uint64_t xsubi = 0;
  uint64_t save_xsubi = 0;
  bool inpass = false;
  bool finished_setup = false;
  bool multitask = false;

  float* hidden_units = nullptr;
  bool* dropped_out = nullptr;

  VW::polyprediction* hidden_units_pred = nullptr;
  VW::polyprediction* hiddenbias_pred = nullptr;

  VW::workspace* all = nullptr;  // many things
  std::shared_ptr<VW::rand_state> random_state;

  ~nn()
  {
    free(hidden_units);
    free(dropped_out);
    free(hidden_units_pred);
    free(hiddenbias_pred);
  }
};

static inline float fastpow2(float p)
{
  float offset = (p < 0) ? 1.0f : 0.0f;
  float clipp = (p < -126) ? -126.0f : p;
  int w = static_cast<int>(clipp);
  float z = clipp - w + offset;
  union
  {
    uint32_t i;
    float f;
  } v = {static_cast<uint32_t>((1 << 23) * (clipp + 121.2740575f + 27.7280233f / (4.84252568f - z) - 1.49012907f * z))};

  return v.f;
}

static inline float fastexp(float p) { return fastpow2(1.442695040f * p); }

static inline float fasttanh(float p) { return -1.0f + 2.0f / (1.0f + fastexp(-2.0f * p)); }

void finish_setup(nn& n, VW::workspace& all)
{
  // TODO: output_layer audit

  n.output_layer.interactions = &all.interactions;
  n.output_layer.extent_interactions = &all.extent_interactions;
  n.output_layer.indices.push_back(VW::details::NN_OUTPUT_NAMESPACE);
  uint64_t nn_index = NN_CONSTANT << all.weights.stride_shift();

  VW::features& fs = n.output_layer.feature_space[VW::details::NN_OUTPUT_NAMESPACE];
  for (unsigned int i = 0; i < n.k; ++i)
  {
    fs.push_back(1., nn_index);
    if (all.audit || all.hash_inv)
    {
      std::stringstream ss;
      ss << "OutputLayer" << i;
      fs.space_names.emplace_back("", ss.str());
    }
    nn_index += static_cast<uint64_t>(n.increment);
  }
  n.output_layer.num_features += n.k;

  if (!n.inpass)
  {
    fs.push_back(1., nn_index);
    if (all.audit || all.hash_inv) { fs.space_names.emplace_back("", "OutputLayerConst"); }
    ++n.output_layer.num_features;
  }

  // TODO: not correct if --noconstant
  n.hiddenbias.interactions = &all.interactions;
  n.hiddenbias.extent_interactions = &all.extent_interactions;
  n.hiddenbias.indices.push_back(VW::details::CONSTANT_NAMESPACE);
  n.hiddenbias.feature_space[VW::details::CONSTANT_NAMESPACE].push_back(1, VW::details::CONSTANT);
  if (all.audit || all.hash_inv)
  {
    n.hiddenbias.feature_space[VW::details::CONSTANT_NAMESPACE].space_names.emplace_back("", "HiddenBias");
  }
  n.hiddenbias.l.simple.label = FLT_MAX;
  n.hiddenbias.weight = 1;

  n.outputweight.interactions = &all.interactions;
  n.outputweight.extent_interactions = &all.extent_interactions;
  n.outputweight.indices.push_back(VW::details::NN_OUTPUT_NAMESPACE);
  VW::features& outfs = n.output_layer.feature_space[VW::details::NN_OUTPUT_NAMESPACE];
  n.outputweight.feature_space[VW::details::NN_OUTPUT_NAMESPACE].push_back(outfs.values[0], outfs.indices[0]);
  if (all.audit || all.hash_inv)
  {
    n.outputweight.feature_space[VW::details::NN_OUTPUT_NAMESPACE].space_names.emplace_back("", "OutputWeight");
  }
  n.outputweight.feature_space[VW::details::NN_OUTPUT_NAMESPACE].values[0] = 1;
  n.outputweight.l.simple.label = FLT_MAX;
  n.outputweight.weight = 1;
  n.outputweight.ex_reduction_features.template get<VW::simple_label_reduction_features>().initial = 0.f;

  n.finished_setup = true;
}

void end_pass(nn& n)
{
  if (n.all->bfgs) { n.xsubi = n.save_xsubi; }
}

template <bool is_learn, bool recompute_hidden>
void predict_or_learn_multi(nn& n, learner& base, VW::example& ec)
{
  bool should_output = n.all->raw_prediction != nullptr;
  if (!n.finished_setup) { finish_setup(n, *(n.all)); }
  // Yes, copy all of shared data.
  VW::shared_data sd{*n.all->sd};
  {
    // guard for all.sd as it is modified - this will restore the state at the end of the scope.
    VW::shared_data* original_sd = n.all->sd.get();
    auto swap_guard = VW::swap_guard(original_sd, &sd);

    VW::simple_label ld = ec.l.simple;
    auto save_set_minmax = n.all->set_minmax;
    float save_min_label;
    float save_max_label;
    float dropscale = n.dropout ? 2.0f : 1.0f;
    auto loss_function_swap_guard = VW::swap_guard(n.all->loss, n.squared_loss);

    VW::polyprediction* hidden_units = n.hidden_units_pred;
    VW::polyprediction* hiddenbias_pred = n.hiddenbias_pred;
    bool* dropped_out = n.dropped_out;

    std::ostringstream output_string_stream;

    n.all->set_minmax = nullptr;
    save_min_label = n.all->sd->min_label;
    n.all->sd->min_label = HIDDEN_MIN_ACTIVATION;
    save_max_label = n.all->sd->max_label;
    n.all->sd->max_label = HIDDEN_MAX_ACTIVATION;

    uint64_t save_ft_offset = ec.ft_offset;

    if (n.multitask) { ec.ft_offset = 0; }

    n.hiddenbias.ft_offset = ec.ft_offset;

    if (recompute_hidden)
    {
      base.multipredict(n.hiddenbias, 0, n.k, hiddenbias_pred, true);

      for (unsigned int i = 0; i < n.k; ++i)
      {
        // avoid saddle point at 0
        if (hiddenbias_pred[i].scalar == 0)
        {
          n.hiddenbias.l.simple.label = static_cast<float>(n.random_state->get_and_update_random() - 0.5);
          base.learn(n.hiddenbias, i);
          n.hiddenbias.l.simple.label = FLT_MAX;
        }
      }

      base.multipredict(ec, 0, n.k, hidden_units, true);

      for (unsigned int i = 0; i < n.k; ++i) { dropped_out[i] = (n.dropout && VW::details::merand48(n.xsubi) < 0.5); }

      if (ec.passthrough)
      {
        for (unsigned int i = 0; i < n.k; ++i)
        {
          VW_ADD_PASSTHROUGH_FEATURE(ec, i * 2, hiddenbias_pred[i].scalar);
          VW_ADD_PASSTHROUGH_FEATURE(ec, i * 2 + 1, hidden_units[i].scalar);
        }
      }
    }

    if (should_output)
    {
      for (unsigned int i = 0; i < n.k; ++i)
      {
        if (i > 0) { output_string_stream << ' '; }
        output_string_stream << i << ':' << hidden_units[i].scalar << ','
                             << fasttanh(hidden_units[i].scalar);  // TODO: huh, what was going on here?
      }
    }

    loss_function_swap_guard.do_swap();
    n.all->set_minmax = save_set_minmax;
    n.all->sd->min_label = save_min_label;
    n.all->sd->max_label = save_max_label;
    ec.ft_offset = save_ft_offset;

    bool converse = false;
    float save_partial_prediction = 0;
    float save_final_prediction = 0;
    float save_ec_loss = 0;

  CONVERSE:  // That's right, I'm using goto.  So sue me.

    n.output_layer.reset_total_sum_feat_sq();
    n.output_layer.feature_space[VW::details::NN_OUTPUT_NAMESPACE].sum_feat_sq = 1;

    n.outputweight.ft_offset = ec.ft_offset;

    n.all->set_minmax = nullptr;
    auto loss_function_swap_guard_converse_block = VW::swap_guard(n.all->loss, n.squared_loss);
    save_min_label = n.all->sd->min_label;
    n.all->sd->min_label = -1;
    save_max_label = n.all->sd->max_label;
    n.all->sd->max_label = 1;

    for (unsigned int i = 0; i < n.k; ++i)
    {
      float sigmah = (dropped_out[i]) ? 0.0f : dropscale * fasttanh(hidden_units[i].scalar);
      VW::features& out_fs = n.output_layer.feature_space[VW::details::NN_OUTPUT_NAMESPACE];
      out_fs.values[i] = sigmah;
      out_fs.sum_feat_sq += sigmah * sigmah;

      n.outputweight.feature_space[VW::details::NN_OUTPUT_NAMESPACE].indices[0] = out_fs.indices[i];
      base.predict(n.outputweight, n.k);
      float wf = n.outputweight.pred.scalar;

      // avoid saddle point at 0
      if (wf == 0)
      {
        float sqrtk = std::sqrt(static_cast<float>(n.k));
        n.outputweight.l.simple.label = static_cast<float>(n.random_state->get_and_update_random() - 0.5) / sqrtk;
        base.update(n.outputweight, n.k);
        n.outputweight.l.simple.label = FLT_MAX;
      }
    }

    loss_function_swap_guard_converse_block.do_swap();
    n.all->set_minmax = save_set_minmax;
    n.all->sd->min_label = save_min_label;
    n.all->sd->max_label = save_max_label;

    if (n.inpass)
    {
      // TODO: this is not correct if there is something in the
      // VW::details::NN_OUTPUT_NAMESPACE but at least it will not leak memory
      // in that case
      ec.indices.push_back(VW::details::NN_OUTPUT_NAMESPACE);

      /*
       * Features shuffling:
       * save_nn_output_namespace contains what was in ec.feature_space[]
       * ec.feature_space[] contains a COPY of n.output_layer.feature_space[]
       * learn/predict is called
       * ec.feature_space[] is reverted to its original value
       * save_nn_output_namespace contains the COPIED value
       * save_nn_output_namespace is destroyed
       */
      VW::features save_nn_output_namespace = std::move(ec.feature_space[VW::details::NN_OUTPUT_NAMESPACE]);
      ec.feature_space[VW::details::NN_OUTPUT_NAMESPACE] =
          n.output_layer.feature_space[VW::details::NN_OUTPUT_NAMESPACE];

      if (is_learn) { base.learn(ec, n.k); }
      else { base.predict(ec, n.k); }
      n.output_layer.partial_prediction = ec.partial_prediction;
      n.output_layer.loss = ec.loss;
      ec.feature_space[VW::details::NN_OUTPUT_NAMESPACE].sum_feat_sq = 0;
      std::swap(ec.feature_space[VW::details::NN_OUTPUT_NAMESPACE], save_nn_output_namespace);
      ec.indices.pop_back();
    }
    else
    {
      n.output_layer.ft_offset = ec.ft_offset;
      n.output_layer.l.simple = ec.l.simple;
      n.output_layer.ex_reduction_features.template get<VW::simple_label_reduction_features>().initial =
          ec.ex_reduction_features.template get<VW::simple_label_reduction_features>().initial;
      n.output_layer.weight = ec.weight;
      n.output_layer.partial_prediction = 0;
      if (is_learn) { base.learn(n.output_layer, n.k); }
      else { base.predict(n.output_layer, n.k); }
    }

    n.prediction = VW::details::finalize_prediction(*n.all->sd, n.all->logger, n.output_layer.partial_prediction);

    if (should_output)
    {
      output_string_stream << ' ' << n.output_layer.partial_prediction;
      n.all->print_text_by_ref(n.all->raw_prediction.get(), output_string_stream.str(), ec.tag, n.all->logger);
    }

    if (is_learn)
    {
      if (n.all->training && ld.label != FLT_MAX)
      {
        float gradient = n.all->loss->first_derivative(n.all->sd.get(), n.prediction, ld.label);

        if (std::fabs(gradient) > 0)
        {
          auto loss_function_swap_guard_learn_block = VW::swap_guard(n.all->loss, n.squared_loss);
          n.all->set_minmax = nullptr;
          save_min_label = n.all->sd->min_label;
          n.all->sd->min_label = HIDDEN_MIN_ACTIVATION;
          save_max_label = n.all->sd->max_label;
          n.all->sd->max_label = HIDDEN_MAX_ACTIVATION;
          save_ft_offset = ec.ft_offset;

          if (n.multitask) { ec.ft_offset = 0; }

          for (unsigned int i = 0; i < n.k; ++i)
          {
            if (!dropped_out[i])
            {
              float sigmah = n.output_layer.feature_space[VW::details::NN_OUTPUT_NAMESPACE].values[i] / dropscale;
              float sigmahprime = dropscale * (1.0f - sigmah * sigmah);
              n.outputweight.feature_space[VW::details::NN_OUTPUT_NAMESPACE].indices[0] =
                  n.output_layer.feature_space[VW::details::NN_OUTPUT_NAMESPACE].indices[i];
              base.predict(n.outputweight, n.k);
              float nu = n.outputweight.pred.scalar;
              float gradhw = 0.5f * nu * gradient * sigmahprime;

              ec.l.simple.label =
                  VW::details::finalize_prediction(*n.all->sd, n.all->logger, hidden_units[i].scalar - gradhw);
              ec.pred.scalar = hidden_units[i].scalar;
              if (ec.l.simple.label != hidden_units[i].scalar) { base.update(ec, i); }
            }
          }

          loss_function_swap_guard_learn_block.do_swap();
          n.all->set_minmax = save_set_minmax;
          n.all->sd->min_label = save_min_label;
          n.all->sd->max_label = save_max_label;
          ec.ft_offset = save_ft_offset;
        }
      }
    }

    ec.l.simple.label = ld.label;

    if (!converse)
    {
      save_partial_prediction = n.output_layer.partial_prediction;
      save_final_prediction = n.prediction;
      save_ec_loss = n.output_layer.loss;
    }

    if (n.dropout && !converse)
    {
      for (unsigned int i = 0; i < n.k; ++i) { dropped_out[i] = !dropped_out[i]; }

      converse = true;
      goto CONVERSE;
    }

    ec.partial_prediction = save_partial_prediction;
    ec.pred.scalar = save_final_prediction;
    ec.loss = save_ec_loss;
  }
  if (n.all->set_minmax)
  {
    n.all->set_minmax(sd.min_label);
    n.all->set_minmax(sd.max_label);
  }
}

void multipredict(nn& n, learner& base, VW::example& ec, size_t count, size_t step, VW::polyprediction* pred,
    bool finalize_predictions)
{
  for (size_t c = 0; c < count; c++)
  {
    if (c == 0) { predict_or_learn_multi<false, true>(n, base, ec); }
    else { predict_or_learn_multi<false, false>(n, base, ec); }
    if (finalize_predictions)
    {
      pred[c] = std::move(ec.pred);  // TODO: this breaks for complex labels because = doesn't do deep copy! (XXX we
                                     // "fix" this by moving)
    }
    else { pred[c].scalar = ec.partial_prediction; }
    ec.ft_offset += static_cast<uint64_t>(step);
  }
  ec.ft_offset -= static_cast<uint64_t>(step * count);
}

// This differs from the simple label based version because nn does not output a raw prediction.
void output_example_prediction_nn(
    VW::workspace& all, const nn& /* data */, const VW::example& ec, VW::io::logger& /* unused */)
{
  for (auto& f : all.final_prediction_sink) { all.print_by_ref(f.get(), ec.pred.scalar, 0, ec.tag, all.logger); }
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::nn_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto n = VW::make_unique<nn>();
  bool meanfield = false;
  option_group_definition new_options("[Reduction] Neural Network");
  new_options
      .add(make_option("nn", n->k).keep().necessary().help("Sigmoidal feedforward network with <k> hidden units"))
      .add(make_option("inpass", n->inpass)
               .keep()
               .help("Train or test sigmoidal feedforward network with input passthrough"))
      .add(make_option("multitask", n->multitask).keep().help("Share hidden layer across all reduced tasks"))
      .add(make_option("dropout", n->dropout).keep().help("Train or test sigmoidal feedforward network using dropout"))
      .add(make_option("meanfield", meanfield).help("Train or test sigmoidal feedforward network using mean field"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  n->all = &all;
  n->random_state = all.get_random_state();

  if (n->multitask && !all.quiet)
  {
    all.logger.err_info("using multitask sharing for neural network {}", (all.training ? "training" : "testing"));
  }

  if (options.was_supplied("meanfield"))
  {
    n->dropout = false;
    all.logger.err_info("using mean field for neural network {}", (all.training ? "training" : "testing"));
  }

  if (n->dropout && !all.quiet)
  {
    all.logger.err_info("using dropout for neural network {}", (all.training ? "training" : "testing"));
  }

  if (n->inpass && !all.quiet)
  {
    all.logger.err_info("using input passthrough for neural network {}", (all.training ? "training" : "testing"));
  }

  n->finished_setup = false;
  n->squared_loss = get_loss_function(all, "squared", 0);

  n->xsubi = all.get_random_state()->get_current_state();

  n->save_xsubi = n->xsubi;

  n->hidden_units = VW::details::calloc_or_throw<float>(n->k);
  n->dropped_out = VW::details::calloc_or_throw<bool>(n->k);
  n->hidden_units_pred = VW::details::calloc_or_throw<VW::polyprediction>(n->k);
  n->hiddenbias_pred = VW::details::calloc_or_throw<VW::polyprediction>(n->k);

  auto base = require_singleline(stack_builder.setup_base_learner());
  n->increment = base->increment;  // Indexing of output layer is odd.
  nn& nv = *n.get();

  size_t ws = n->k + 1;

  auto builder = make_reduction_learner(std::move(n), base, predict_or_learn_multi<true, true>,
      predict_or_learn_multi<false, true>, stack_builder.get_setupfn_name(nn_setup))
                     .set_params_per_weight(ws)
                     .set_learn_returns_prediction(true)
                     .set_input_prediction_type(VW::prediction_type_t::SCALAR)
                     .set_output_prediction_type(VW::prediction_type_t::SCALAR)
                     .set_input_label_type(VW::label_type_t::SIMPLE)
                     .set_output_label_type(VW::label_type_t::SIMPLE)
                     .set_output_example_prediction(output_example_prediction_nn)
                     .set_print_update(VW::details::print_update_simple_label<nn>)
                     .set_update_stats(VW::details::update_stats_simple_label<nn>)
                     .set_end_pass(end_pass);

  if (nv.multitask) { builder.set_multipredict(multipredict); }

  return builder.build();
}

/*

  train: ./vw -k -c -d mnist8v9.gz --passes 24 -b 25 --nn 64 -l 0.1 --invariant --adaptive --holdout_off --random_seed
19 --nnmultipredict -f mnist64 predict: ./vw -t -d mnist8v9.gz -i mnist64 --nnmultipredict

                     default   multipredict
  nn  64 train         9.1s         8.1s
         predict       0.57s        0.52s
  nn 128 train        16.5s        13.8s
         predict       0.76s        0.69s

with oaa:

  train: ./vw --oaa 10 -b 25 --adaptive --invariant --holdout_off -l 0.1 --nn 64 --passes 24 -k -c -d mnist-all.gz
--random_seed 19 --nnmultipredict -f mnist-all64 predict: ./vw -t -d mnist-all.gz -i mnist-all64 --nnmultipredict

*/
