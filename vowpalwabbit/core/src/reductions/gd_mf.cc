// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "vw/core/reductions/gd_mf.h"

#include "vw/common/random_details.h"
#include "vw/common/vw_exception.h"
#include "vw/core/array_parameters.h"
#include "vw/core/crossplat_compat.h"
#include "vw/core/learner.h"
#include "vw/core/loss_functions.h"
#include "vw/core/parse_regressor.h"
#include "vw/core/parser.h"
#include "vw/core/prediction_type.h"
#include "vw/core/reductions/gd.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/simple_label.h"

#include <cfloat>
#include <cstdio>
#include <fstream>

using namespace VW::LEARNER;
using namespace VW::config;

namespace
{
class gdmf
{
public:
  VW::workspace* all = nullptr;  // regressor, printing
  VW::v_array<float> scalars;
  uint32_t rank = 0;
  size_t no_win_counter = 0;
  uint64_t early_stop_thres = 0;
};

void mf_print_offset_features(gdmf& d, VW::example& ec, size_t offset)
{
  // TODO: Where should audit stuff output to?
  VW::workspace& all = *d.all;
  auto& weights = all.weights;
  uint64_t mask = weights.mask();
  for (VW::features& fs : ec)
  {
    bool audit = !fs.space_names.empty();
    for (const auto& f : fs.audit_range())
    {
      std::cout << '\t';
      if (audit) { std::cout << VW::to_string(*f.audit()) << ':'; }
      std::cout << f.index() << "(" << ((f.index() + offset) & mask) << ")" << ':' << f.value();
      std::cout << ':' << (&weights[f.index()])[offset];
    }
  }
  for (const auto& i : all.interactions)
  {
    if (i.size() != 2) THROW("can only use pairs in matrix factorization");

    if (ec.feature_space[static_cast<unsigned char>(i[0])].size() > 0 &&
        ec.feature_space[static_cast<unsigned char>(i[1])].size() > 0)
    {
      /* print out nsk^feature:hash:value:weight:nsk^feature^:hash:value:weight:prod_weights */
      for (size_t k = 1; k <= d.rank; k++)
      {
        for (const auto& f1 : ec.feature_space[static_cast<unsigned char>(i[0])].audit_range())
        {
          for (const auto& f2 : ec.feature_space[static_cast<unsigned char>(i[1])].audit_range())
          {
            std::cout << '\t' << VW::to_string(*f1.audit()) << ':' << ((f1.index() + k) & mask) << "("
                      << ((f1.index() + offset + k) & mask) << ")" << ':' << f1.value();
            std::cout << ':' << (&weights[f1.index()])[offset + k];

            std::cout << ':' << VW::to_string(*f2.audit()) << ':' << ((f2.index() + k + d.rank) & mask) << "("
                      << ((f2.index() + offset + k + d.rank) & mask) << ")" << ':' << f2.value();
            std::cout << ':' << (&weights[f2.index()])[offset + k + d.rank];

            std::cout << ':' << (&weights[f1.index()])[offset + k] * (&weights[f2.index()])[offset + k + d.rank];
          }
        }
      }
    }
  }
  std::cout << std::endl;
}

void mf_print_audit_features(gdmf& d, VW::example& ec, size_t offset)
{
  VW::details::print_result_by_ref(d.all->stdout_adapter.get(), ec.pred.scalar, -1, ec.tag, d.all->logger);
  mf_print_offset_features(d, ec, offset);
}

class pred_offset
{
public:
  float p;
  uint64_t offset;
};

void offset_add(pred_offset& res, const float fx, float& fw) { res.p += (&fw)[res.offset] * fx; }

template <class T>
float mf_predict(gdmf& d, VW::example& ec, T& weights)
{
  VW::workspace& all = *d.all;
  const auto& simple_red_features = ec.ex_reduction_features.template get<VW::simple_label_reduction_features>();
  float prediction = simple_red_features.initial;

  ec.num_features_from_interactions = 0;
  for (const auto& i : d.all->interactions)
  {
    if (i.size() != 2) THROW("can only use pairs in matrix factorization");
    const auto interacted_count =
        ec.feature_space[static_cast<int>(i[0])].size() * ec.feature_space[static_cast<int>(i[1])].size();
    ec.num_features -= interacted_count;
    ec.num_features += ec.feature_space[static_cast<int>(i[0])].size() * d.rank;
    ec.num_features += ec.feature_space[static_cast<int>(i[1])].size() * d.rank;
    ec.num_features_from_interactions += interacted_count;
  }

  // clear stored predictions
  d.scalars.clear();

  float linear_prediction = 0.;
  // linear terms

  for (VW::features& fs : ec) { VW::foreach_feature<float, VW::details::vec_add, T>(weights, fs, linear_prediction); }

  // store constant + linear prediction
  // note: constant is now automatically added
  d.scalars.push_back(linear_prediction);

  prediction += linear_prediction;
  // interaction terms
  for (const auto& i : d.all->interactions)
  {
    // The check for non-pair interactions is done in the previous loop

    if (ec.feature_space[static_cast<int>(i[0])].size() > 0 && ec.feature_space[static_cast<int>(i[1])].size() > 0)
    {
      for (uint64_t k = 1; k <= d.rank; k++)
      {
        // x_l * l^k
        // l^k is from index+1 to index+d.rank
        // float x_dot_l = sd_offset_add(weights, ec.atomics[(int)(*i)[0]].begin(), ec.atomics[(int)(*i)[0]].end(), k);
        pred_offset x_dot_l = {0., k};
        VW::foreach_feature<pred_offset, offset_add, T>(weights, ec.feature_space[static_cast<int>(i[0])], x_dot_l);
        // x_r * r^k
        // r^k is from index+d.rank+1 to index+2*d.rank
        // float x_dot_r = sd_offset_add(weights, ec.atomics[(int)(*i)[1]].begin(), ec.atomics[(int)(*i)[1]].end(),
        // k+d.rank);
        pred_offset x_dot_r = {0., k + d.rank};
        VW::foreach_feature<pred_offset, offset_add, T>(weights, ec.feature_space[static_cast<int>(i[1])], x_dot_r);

        prediction += x_dot_l.p * x_dot_r.p;

        // store prediction from interaction terms
        d.scalars.push_back(x_dot_l.p);
        d.scalars.push_back(x_dot_r.p);
      }
    }
  }

  // d.scalars has linear, x_dot_l_1, x_dot_r_1, x_dot_l_2, x_dot_r_2, ...

  ec.partial_prediction = prediction;

  if (all.set_minmax) { all.set_minmax(ec.l.simple.label); }

  ec.pred.scalar = VW::details::finalize_prediction(*all.sd, all.logger, ec.partial_prediction);

  if (ec.l.simple.label != FLT_MAX)
  {
    ec.loss = all.loss->get_loss(all.sd.get(), ec.pred.scalar, ec.l.simple.label) * ec.weight;
  }

  if (all.audit) { mf_print_audit_features(d, ec, 0); }

  return ec.pred.scalar;
}

float mf_predict(gdmf& d, VW::example& ec)
{
  VW::workspace& all = *d.all;
  if (all.weights.sparse) { return mf_predict(d, ec, all.weights.sparse_weights); }
  else { return mf_predict(d, ec, all.weights.dense_weights); }
}

template <class T>
void sd_offset_update(T& weights, VW::features& fs, uint64_t offset, float update, float regularization)
{
  for (size_t i = 0; i < fs.size(); i++)
  {
    (&weights[fs.indices[i]])[offset] += update * fs.values[i] - regularization * (&weights[fs.indices[i]])[offset];
  }
}

template <class T>
void mf_train(gdmf& d, VW::example& ec, T& weights)
{
  VW::workspace& all = *d.all;
  VW::simple_label& ld = ec.l.simple;

  // use final prediction to get update size
  // update = eta_t*(y-y_hat) where eta_t = eta/(3*t^p) * importance weight
  float eta_t = all.eta / powf(static_cast<float>(all.sd->t) + ec.weight, all.power_t) / 3.f * ec.weight;
  float update = all.loss->get_update(ec.pred.scalar, ld.label, eta_t, 1.);  // ec.total_sum_feat_sq);

  float regularization = eta_t * all.l2_lambda;

  // linear update
  for (VW::features& fs : ec) { sd_offset_update<T>(weights, fs, 0, update, regularization); }

  // quadratic update
  for (const auto& i : all.interactions)
  {
    if (i.size() != 2) THROW("can only use pairs in matrix factorization");

    if (ec.feature_space[static_cast<int>(i[0])].size() > 0 && ec.feature_space[static_cast<int>(i[1])].size() > 0)
    {
      // update l^k weights
      for (size_t k = 1; k <= d.rank; k++)
      {
        // r^k \cdot x_r
        float r_dot_x = d.scalars[2 * k];
        // l^k <- l^k + update * (r^k \cdot x_r) * x_l
        sd_offset_update<T>(weights, ec.feature_space[static_cast<int>(i[0])], k, update * r_dot_x, regularization);
      }
      // update r^k weights
      for (size_t k = 1; k <= d.rank; k++)
      {
        // l^k \cdot x_l
        float l_dot_x = d.scalars[2 * k - 1];
        // r^k <- r^k + update * (l^k \cdot x_l) * x_r
        sd_offset_update<T>(
            weights, ec.feature_space[static_cast<int>(i[1])], k + d.rank, update * l_dot_x, regularization);
      }
    }
  }
}

void mf_train(gdmf& d, VW::example& ec)
{
  if (d.all->weights.sparse) { mf_train(d, ec, d.all->weights.sparse_weights); }
  else { mf_train(d, ec, d.all->weights.dense_weights); }
}

void initialize_weights(VW::weight* weights, uint64_t index, uint32_t stride)
{
  for (size_t i = 0; i != stride; ++i, ++index)
  {
    float initial_value = 0.1f * VW::details::merand48(index);
    weights[i] = initial_value;
  }
}

void save_load(gdmf& d, VW::io_buf& model_file, bool read, bool text)
{
  VW::workspace& all = *d.all;
  uint64_t length = static_cast<uint64_t>(1) << all.num_bits;
  if (read)
  {
    VW::details::initialize_regressor(all);
    if (all.random_weights)
    {
      uint32_t stride = all.weights.stride();
      auto weight_initializer = [stride](VW::weight* weights, uint64_t index)
      { initialize_weights(weights, index, stride); };

      all.weights.set_default(weight_initializer);
    }
  }

  if (model_file.num_files() > 0)
  {
    if (!all.weights.not_null())
    {
      THROW("Model weights object was not initialized when trying to data load into it.");
    }
    uint64_t i = 0;
    size_t brw = 1;
    do {
      brw = 0;
      size_t K = d.rank * 2 + 1;  // NOLINT
      std::stringstream msg;
      msg << i << " ";
      brw +=
          VW::details::bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&i), sizeof(i), read, msg, text);
      if (brw != 0)
      {
        VW::weight* w_i = &(all.weights.strided_index(i));
        for (uint64_t k = 0; k < K; k++)
        {
          VW::weight* v = w_i + k;
          msg << v << " ";
          brw += VW::details::bin_text_read_write_fixed(
              model_file, reinterpret_cast<char*>(v), sizeof(*v), read, msg, text);
        }
      }
      if (text)
      {
        msg << "\n";
        brw += VW::details::bin_text_read_write_fixed(model_file, nullptr, 0, read, msg, text);
      }

      if (!read) { ++i; }
    } while ((!read && i < length) || (read && brw > 0));
  }
}

void end_pass(gdmf& d)
{
  VW::workspace* all = d.all;

  all->eta *= all->eta_decay_rate;
  if (all->save_per_pass) { VW::details::save_predictor(*all, all->final_regressor_name, all->current_pass); }

  if (!all->holdout_set_off)
  {
    if (VW::details::summarize_holdout_set(*all, d.no_win_counter))
    {
      VW::details::finalize_regressor(*all, all->final_regressor_name);
    }
    if ((d.early_stop_thres == d.no_win_counter) &&
        ((all->check_holdout_every_n_passes <= 1) || ((all->current_pass % all->check_holdout_every_n_passes) == 0)))
    {
      VW::details::set_done(*all);
    }
  }
}

void predict(gdmf& d, VW::example& ec) { mf_predict(d, ec); }

void learn(gdmf& d, VW::example& ec)
{
  VW::workspace& all = *d.all;

  mf_predict(d, ec);
  if (all.training && ec.l.simple.label != FLT_MAX) { mf_train(d, ec); }
}

}  // namespace
std::shared_ptr<VW::LEARNER::learner> VW::reductions::gd_mf_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();

  auto data = VW::make_unique<gdmf>();

  option_group_definition gf_md_options("[Reduction] Gradient Descent Matrix Factorization");
  gf_md_options.add(make_option("rank", data->rank).keep().necessary().help("Rank for matrix factorization"));

  if (!options.add_parse_and_check_necessary(gf_md_options)) { return nullptr; }

  if (options.was_supplied("adaptive")) THROW("adaptive is not implemented for matrix factorization");
  if (options.was_supplied("normalized")) THROW("normalized is not implemented for matrix factorization");
  if (options.was_supplied("exact_adaptive_norm"))
    THROW("normalized adaptive updates is not implemented for matrix factorization");

  if (options.was_supplied("bfgs") || options.was_supplied("conjugate_gradient"))
  {
    THROW("bfgs is not implemented for matrix factorization");
  }

  data->all = &all;
  data->no_win_counter = 0;

  // store linear + 2*rank weights per index, round up to power of two
  float temp = ceilf(logf(static_cast<float>(data->rank * 2 + 1)) / logf(2.f));
  all.weights.stride_shift(static_cast<size_t>(temp));
  all.random_weights = true;

  if (!all.holdout_set_off)
  {
    all.sd->holdout_best_loss = FLT_MAX;
    data->early_stop_thres = options.get_typed_option<uint64_t>("early_terminate").value();
  }

  if (!options.was_supplied("learning_rate") && !options.was_supplied("l"))
  {
    all.eta = 10;  // default learning rate to 10 for non default update rule
  }

  // default initial_t to 1 instead of 0
  if (!options.was_supplied("initial_t"))
  {
    all.sd->t = 1.f;
    all.initial_t = 1.f;
  }
  all.eta *= powf(static_cast<float>(all.sd->t), all.power_t);

  auto l = make_bottom_learner(std::move(data), learn, predict, stack_builder.get_setupfn_name(gd_mf_setup),
      VW::prediction_type_t::SCALAR, VW::label_type_t::SIMPLE)
               .set_params_per_weight(VW::details::UINT64_ONE << all.weights.stride_shift())
               .set_learn_returns_prediction(true)
               .set_save_load(save_load)
               .set_end_pass(end_pass)
               .set_output_example_prediction(VW::details::output_example_prediction_simple_label<gdmf>)
               .set_update_stats(VW::details::update_stats_simple_label<gdmf>)
               .set_print_update(VW::details::print_update_simple_label<gdmf>)
               .build();

  return l;
}
