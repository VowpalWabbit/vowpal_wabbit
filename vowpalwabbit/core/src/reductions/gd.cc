// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/array_parameters.h"
#include "vw/core/array_parameters_dense.h"
#include "vw/core/crossplat_compat.h"
#include "vw/core/feature_group.h"
#include "vw/core/global_data.h"
#include "vw/core/learner.h"
#include "vw/core/loss_functions.h"
#include "vw/core/prediction_type.h"
#include "vw/core/setup_base.h"

#include <algorithm>
#include <cfloat>

#if !defined(VW_NO_INLINE_SIMD)
#  if !defined(__SSE2__) && (defined(_M_AMD64) || defined(_M_X64))
#    define __SSE2__
#  endif

#  if defined(__ARM_NEON__)
#    include <arm_neon.h>
#  elif defined(__SSE2__)
#    include <xmmintrin.h>
#  endif
#endif

#include "vw/core/accumulate.h"
#include "vw/core/debug_log.h"
#include "vw/core/label_parser.h"
#include "vw/core/parse_regressor.h"
#include "vw/core/reductions/gd.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/core/vw_versions.h"

#undef VW_DEBUG_LOG
#define VW_DEBUG_LOG vw_dbg::GD
#include "vw/io/logger.h"

using namespace VW::LEARNER;
using namespace VW::config;

namespace
{
constexpr double L1_STATE_DEFAULT = 0.;
constexpr double L2_STATE_DEFAULT = 1.;

template <typename WeightsT>
void merge_weights_simple(size_t length, const std::vector<std::reference_wrapper<const WeightsT>>& source,
    const std::vector<float>& per_model_weighting, WeightsT& weights)
{
  for (size_t i = 0; i < source.size(); i++)
  {
    const auto& this_source = source[i].get();
    for (size_t j = 0; j < length; j++)
    {
      weights.strided_index(j) += (this_source.strided_index(j) * per_model_weighting[i]);
    }
  }
}

void merge_weights_with_save_resume(size_t length,
    const std::vector<std::reference_wrapper<const VW::dense_parameters>>& source,
    const std::vector<float>& /*per_model_weighting*/, VW::workspace& output_workspace, VW::dense_parameters& weights)
{
  // Adaptive totals
  std::vector<float> adaptive_totals(length, 0.f);
  for (const auto& model : source)
  {
    const auto& this_model = model.get();
    for (size_t i = 0; i < length; i++) { adaptive_totals[i] += (&(this_model[i << weights.stride_shift()]))[1]; }
  }

  std::vector<VW::dense_parameters> weight_copies;
  weight_copies.reserve(source.size());
  for (auto i : source)
  {
    // There is no copy constructor for weights, so we have to copy manually.
    weight_copies.emplace_back(VW::dense_parameters::deep_copy(i));
    VW::details::do_weighting(output_workspace.normalized_idx, length, adaptive_totals.data(), weight_copies.back());
  }

  // Weights have already been reweighted, so just accumulate.
  for (const auto& this_source : weight_copies)
  {
    // Intentionally add irrespective of stride.
    const auto full_weights_size = length << weights.stride_shift();
    for (uint64_t i = 0; i < full_weights_size; i++) { weights[i] += this_source[i]; }
  }
}

template <typename WeightsT>
void add_weights(WeightsT& dest, const WeightsT& lhs, const WeightsT& rhs, size_t length)
{
  const size_t full_weights_size = length << dest.stride_shift();
  for (size_t i = 0; i < full_weights_size; i++) { dest[i] = lhs[i] + rhs[i]; }
}

template <typename WeightsT>
void subtract_weights(WeightsT& dest, const WeightsT& lhs, const WeightsT& rhs, size_t length)
{
  const size_t full_weights_size = length << dest.stride_shift();
  for (size_t i = 0; i < full_weights_size; i++) { dest[i] = lhs[i] - rhs[i]; }
}

void sync_weights(VW::workspace& all)
{
  // todo, fix length dependence
  if (all.sd->gravity == 0. && all.sd->contraction == 1.)
  {  // to avoid unnecessary weight synchronization
    return;
  }

  if (all.weights.sparse)
  {
    for (VW::weight& w : all.weights.sparse_weights)
    {
      w = VW::trunc_weight(w, static_cast<float>(all.sd->gravity)) * static_cast<float>(all.sd->contraction);
    }
  }
  else
  {
    for (VW::weight& w : all.weights.dense_weights)
    {
      w = VW::trunc_weight(w, static_cast<float>(all.sd->gravity)) * static_cast<float>(all.sd->contraction);
    }
  }

  all.sd->gravity = 0.;
  all.sd->contraction = 1.;
}

VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_UNUSED_FUNCTION
inline float quake_inv_sqrt(float x)
{
  // Carmack/Quake/SGI fast method:
  float xhalf = 0.5f * x;
  static_assert(sizeof(int) == sizeof(float), "Floats and ints are converted between, they must be the same size.");
  int i = reinterpret_cast<int&>(x);  // store floating-point bits in integer
  i = 0x5f3759d5 - (i >> 1);          // initial guess for Newton's method
  x = reinterpret_cast<float&>(i);    // convert new bits into float
  x = x * (1.5f - xhalf * x * x);     // One round of Newton's method
  return x;
}
VW_WARNING_STATE_POP

static inline float inv_sqrt(float x)
{
#if !defined(VW_NO_INLINE_SIMD)
#  if defined(__ARM_NEON__)
  // Propagate into vector
  float32x2_t v1 = vdup_n_f32(x);
  // Estimate
  float32x2_t e1 = vrsqrte_f32(v1);
  // N-R iteration 1
  float32x2_t e2 = vmul_f32(e1, vrsqrts_f32(v1, vmul_f32(e1, e1)));
  // N-R iteration 2
  float32x2_t e3 = vmul_f32(e2, vrsqrts_f32(v1, vmul_f32(e2, e2)));
  // Extract result
  return vget_lane_f32(e3, 0);
#  elif defined(__SSE2__)
  __m128 eta = _mm_load_ss(&x);
  eta = _mm_rsqrt_ss(eta);
  _mm_store_ss(&x, eta);
#  else
  x = quake_inv_sqrt(x);
#  endif
#else
  x = quake_inv_sqrt(x);
#endif

  return x;
}

VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_COND_CONST_EXPR
template <bool sqrt_rate, bool feature_mask_off, size_t adaptive, size_t normalized, size_t spare>
inline void update_feature(float& update, float x, float& fw)
{
  VW::weight* w = &fw;
  bool modify = x < FLT_MAX && x > -FLT_MAX && (feature_mask_off || fw != 0.);
  if (modify)
  {
    if VW_STD17_CONSTEXPR (spare != 0) { x *= w[spare]; }
    w[0] += update * x;
  }
}

// this deals with few nonzero features vs. all nonzero features issues.
template <bool sqrt_rate, size_t adaptive, size_t normalized>
float average_update(float total_weight, float normalized_sum_norm_x, float neg_norm_power)
{
  if VW_STD17_CONSTEXPR (normalized != 0)
  {
    if (sqrt_rate)
    {
      float avg_norm = (total_weight / normalized_sum_norm_x);
      if (adaptive) { return std::sqrt(avg_norm); }
      else { return avg_norm; }
    }
    else { return powf((normalized_sum_norm_x / total_weight), neg_norm_power); }
  }
  return 1.f;
}

template <bool sqrt_rate, bool feature_mask_off, size_t adaptive, size_t normalized, size_t spare>
void train(VW::reductions::gd& g, VW::example& ec, float update)
{
  if VW_STD17_CONSTEXPR (normalized != 0) { update *= g.update_multiplier; }
  VW_DBG(ec) << "gd: train() spare=" << spare << std::endl;
  VW::foreach_feature<float, update_feature<sqrt_rate, feature_mask_off, adaptive, normalized, spare>>(
      *g.all, ec, update);
}

void end_pass(VW::reductions::gd& g)
{
  VW::workspace& all = *g.all;

  if (!all.save_resume) { sync_weights(all); }

  if (all.all_reduce != nullptr)
  {
    if (all.weights.adaptive) { VW::details::accumulate_weighted_avg(all, all.weights); }
    else { VW::details::accumulate_avg(all, all.weights, 0); }
  }
  all.eta *= all.eta_decay_rate;
  if (all.save_per_pass) { VW::details::save_predictor(all, all.final_regressor_name, all.current_pass); }

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

void merge(const std::vector<float>& per_model_weighting, const std::vector<const VW::workspace*>& all_workspaces,
    const std::vector<const VW::reductions::gd*>& all_data, VW::workspace& output_workspace,
    VW::reductions::gd& output_data)
{
  const size_t length = static_cast<size_t>(1) << output_workspace.num_bits;

  // Weight aggregation is based on same method as allreduce.
  if (output_workspace.weights.sparse)
  {
    std::vector<std::reference_wrapper<const VW::sparse_parameters>> source;
    source.reserve(all_workspaces.size());
    for (const auto* workspace : all_workspaces) { source.emplace_back(workspace->weights.sparse_weights); }
    if (output_workspace.weights.adaptive) { THROW("Sparse parameters not supported for merging with save_resume"); }
    else { merge_weights_simple(length, source, per_model_weighting, output_workspace.weights.sparse_weights); }
  }
  else
  {
    std::vector<std::reference_wrapper<const VW::dense_parameters>> source;
    source.reserve(all_workspaces.size());
    for (const auto* workspace : all_workspaces) { source.emplace_back(workspace->weights.dense_weights); }
    if (output_workspace.weights.adaptive)
    {
      merge_weights_with_save_resume(
          length, source, per_model_weighting, output_workspace, output_workspace.weights.dense_weights);
    }
    else { merge_weights_simple(length, source, per_model_weighting, output_workspace.weights.dense_weights); }
  }

  for (size_t i = 0; i < output_data.per_model_states.size(); i++)
  {
    for (const auto* source_data_obj : all_data)
    {
      // normalized_sum_norm_x is additive
      output_data.per_model_states[i].normalized_sum_norm_x +=
          source_data_obj->per_model_states[i].normalized_sum_norm_x;
      // total_weight is additive
      output_data.per_model_states[i].total_weight += source_data_obj->per_model_states[i].total_weight;
    }
  }
}

void add(const VW::workspace& ws1, const VW::reductions::gd& data1, const VW::workspace& ws2,
    const VW::reductions::gd& data2, VW::workspace& ws_out, VW::reductions::gd& data_out)
{
  const size_t length = static_cast<size_t>(1) << ws_out.num_bits;
  if (ws_out.weights.sparse)
  {
    add_weights(ws_out.weights.sparse_weights, ws1.weights.sparse_weights, ws2.weights.sparse_weights, length);
  }
  else { add_weights(ws_out.weights.dense_weights, ws1.weights.dense_weights, ws2.weights.dense_weights, length); }

  for (size_t i = 0; i < data_out.per_model_states.size(); i++)
  {
    // normalized_sum_norm_x is additive
    data_out.per_model_states[i].normalized_sum_norm_x =
        data1.per_model_states[i].normalized_sum_norm_x + data2.per_model_states[i].normalized_sum_norm_x;
    // total_weight is additive
    data_out.per_model_states[i].total_weight =
        data1.per_model_states[i].total_weight + data2.per_model_states[i].total_weight;
  }
}

void subtract(const VW::workspace& ws1, const VW::reductions::gd& data1, const VW::workspace& ws2,
    const VW::reductions::gd& data2, VW::workspace& ws_out, VW::reductions::gd& data_out)
{
  const size_t length = static_cast<size_t>(1) << ws_out.num_bits;
  if (ws_out.weights.sparse)
  {
    subtract_weights(ws_out.weights.sparse_weights, ws1.weights.sparse_weights, ws2.weights.sparse_weights, length);
  }
  else { subtract_weights(ws_out.weights.dense_weights, ws1.weights.dense_weights, ws2.weights.dense_weights, length); }

  for (size_t i = 0; i < data_out.per_model_states.size(); i++)
  {
    // normalized_sum_norm_x is additive
    data_out.per_model_states[i].normalized_sum_norm_x =
        data1.per_model_states[i].normalized_sum_norm_x - data2.per_model_states[i].normalized_sum_norm_x;
    // total_weight is additive
    data_out.per_model_states[i].total_weight =
        data1.per_model_states[i].total_weight - data2.per_model_states[i].total_weight;
  }
}

class string_value
{
public:
  float v;
  std::string s;
  friend bool operator<(const string_value& first, const string_value& second);
};

bool operator<(const string_value& first, const string_value& second) { return fabsf(first.v) > fabsf(second.v); }

class audit_results
{
public:
  VW::workspace& all;
  const uint64_t offset;
  std::vector<VW::audit_strings> components;
  std::vector<string_value> results;
  audit_results(VW::workspace& p_all, const size_t p_offset) : all(p_all), offset(p_offset) {}
};

inline void audit_interaction(audit_results& dat, const VW::audit_strings* f)
{
  if (f == nullptr)
  {
    if (!dat.components.empty()) { dat.components.pop_back(); }

    return;
  }
  if (!f->is_empty()) { dat.components.push_back(*f); }
}

inline void audit_feature(audit_results& dat, const float ft_weight, const uint64_t ft_idx)
{
  auto& weights = dat.all.weights;
  uint64_t index = ft_idx & weights.mask();
  size_t stride_shift = weights.stride_shift();

  std::ostringstream tempstream;
  for (size_t i = 0; i < dat.components.size(); i++)
  {
    if (i > 0) { tempstream << "*"; }
    tempstream << VW::to_string(dat.components[i]);
  }

  if (dat.all.audit)
  {
    tempstream << ':' << (index >> stride_shift) << ':' << ft_weight << ':'
               << VW::trunc_weight(weights[index], static_cast<float>(dat.all.sd->gravity)) *
            static_cast<float>(dat.all.sd->contraction);

    if (weights.adaptive)
    {  // adaptive
      tempstream << '@' << (&weights[index])[1];
    }

    string_value sv = {weights[index] * ft_weight, tempstream.str()};
    dat.results.push_back(sv);
  }

  if ((dat.all.current_pass == 0 || dat.all.training == false) && dat.all.hash_inv)
  {
    const auto strided_index = index >> stride_shift;
    if (dat.all.index_name_map.count(strided_index) == 0)
    {
      VW::details::invert_hash_info info;
      info.weight_components = dat.components;
      info.offset = dat.offset;
      info.stride_shift = stride_shift;
      dat.all.index_name_map.insert(std::make_pair(strided_index, info));
    }
  }
}
void print_lda_features(VW::workspace& all, VW::example& ec)
{
  VW::parameters& weights = all.weights;
  uint32_t stride_shift = weights.stride_shift();
  size_t count = 0;
  for (VW::features& fs : ec) { count += fs.size(); }
  // TODO: Where should audit stuff output to?
  for (VW::features& fs : ec)
  {
    for (const auto& f : fs.audit_range())
    {
      std::cout << '\t' << VW::to_string(*f.audit()) << ':' << ((f.index() >> stride_shift) & all.parse_mask) << ':'
                << f.value();
      for (size_t k = 0; k < all.lda; k++) { std::cout << ':' << (&weights[f.index()])[k]; }
    }
  }
  std::cout << " total of " << count << " features." << std::endl;
}
}  // namespace

void VW::details::print_features(VW::workspace& all, VW::example& ec)
{
  if (all.lda > 0) { print_lda_features(all, ec); }
  else
  {
    audit_results dat(all, ec.ft_offset);

    for (VW::features& fs : ec)
    {
      if (fs.space_names.size() > 0)
      {
        for (const auto& f : fs.audit_range())
        {
          audit_interaction(dat, f.audit());
          audit_feature(dat, f.value(), f.index() + ec.ft_offset);
          audit_interaction(dat, nullptr);
        }
      }
      else
      {
        for (const auto& f : fs) { audit_feature(dat, f.value(), f.index() + ec.ft_offset); }
      }
    }
    size_t num_interacted_features = 0;
    VW::generate_interactions<audit_results, const uint64_t, audit_feature, true, audit_interaction>(
        all, ec, dat, num_interacted_features);

    stable_sort(dat.results.begin(), dat.results.end());
    if (all.audit)
    {
      for (string_value& sv : dat.results)
      {
        all.audit_writer->write("\t", 1);
        all.audit_writer->write(sv.s.data(), sv.s.size());
      }
      all.audit_writer->write("\n", 1);
    }
  }
}

void VW::details::print_audit_features(VW::workspace& all, VW::example& ec)
{
  if (all.audit) { VW::details::print_result_by_ref(all.audit_writer.get(), ec.pred.scalar, -1, ec.tag, all.logger); }
  fflush(stdout);
  print_features(all, ec);
}

float VW::details::finalize_prediction(VW::shared_data& sd, VW::io::logger& logger, float ret)
{
  if (std::isnan(ret))
  {
    ret = 0.;
    logger.err_warn("NAN prediction in example {0}, forcing {1}", sd.example_number + 1, ret);
    return ret;
  }
  if (ret > sd.max_label) { return sd.max_label; }
  if (ret < sd.min_label) { return sd.min_label; }
  return ret;
}

namespace
{
class trunc_data
{
public:
  float prediction;
  float gravity;
};

inline void vec_add_trunc(trunc_data& p, const float fx, float& fw)
{
  p.prediction += VW::trunc_weight(fw, p.gravity) * fx;
}

inline float trunc_predict(VW::workspace& all, VW::example& ec, double gravity, size_t& num_interacted_features)
{
  const auto& simple_red_features = ec.ex_reduction_features.template get<VW::simple_label_reduction_features>();
  trunc_data temp = {simple_red_features.initial, static_cast<float>(gravity)};
  VW::foreach_feature<trunc_data, vec_add_trunc>(all, ec, temp, num_interacted_features);
  return temp.prediction;
}

template <bool l1, bool audit>
void predict(VW::reductions::gd& g, VW::example& ec)
{
  VW_DBG(ec) << "gd.predict(): ex#=" << ec.example_counter << ", offset=" << ec.ft_offset << std::endl;

  VW::workspace& all = *g.all;
  size_t num_interacted_features = 0;
  if (l1) { ec.partial_prediction = trunc_predict(all, ec, all.sd->gravity, num_interacted_features); }
  else { ec.partial_prediction = inline_predict(all, ec, num_interacted_features); }

  ec.num_features_from_interactions = num_interacted_features;
  ec.partial_prediction *= static_cast<float>(all.sd->contraction);
  ec.pred.scalar = VW::details::finalize_prediction(*all.sd, all.logger, ec.partial_prediction);

  VW_DBG(ec) << "gd: predict() " << VW::debug::scalar_pred_to_string(ec) << VW::debug::features_to_string(ec)
             << std::endl;

  if (audit) { VW::details::print_audit_features(all, ec); }
}

template <class T>
inline void vec_add_trunc_multipredict(VW::details::multipredict_info<T>& mp, const float fx, uint64_t fi)
{
  size_t index = fi;
  for (size_t c = 0; c < mp.count; c++, index += mp.step)
  {
    mp.pred[c].scalar += fx * VW::trunc_weight(mp.weights[index], mp.gravity);
  }
}

template <bool l1, bool audit>
void multipredict(VW::reductions::gd& g, VW::example& ec, size_t count, size_t step, VW::polyprediction* pred,
    bool finalize_predictions)
{
  VW::workspace& all = *g.all;
  for (size_t c = 0; c < count; c++)
  {
    const auto& simple_red_features = ec.ex_reduction_features.template get<VW::simple_label_reduction_features>();
    pred[c].scalar = simple_red_features.initial;
  }

  size_t num_features_from_interactions = 0;
  if (g.all->weights.sparse)
  {
    VW::details::multipredict_info<VW::sparse_parameters> mp = {
        count, step, pred, g.all->weights.sparse_weights, static_cast<float>(all.sd->gravity)};
    if (l1)
    {
      VW::foreach_feature<VW::details::multipredict_info<VW::sparse_parameters>, uint64_t, vec_add_trunc_multipredict>(
          all, ec, mp, num_features_from_interactions);
    }
    else
    {
      VW::foreach_feature<VW::details::multipredict_info<VW::sparse_parameters>, uint64_t,
          VW::details::vec_add_multipredict>(all, ec, mp, num_features_from_interactions);
    }
  }
  else
  {
    VW::details::multipredict_info<VW::dense_parameters> mp = {
        count, step, pred, g.all->weights.dense_weights, static_cast<float>(all.sd->gravity)};
    if (l1)
    {
      VW::foreach_feature<VW::details::multipredict_info<VW::dense_parameters>, uint64_t, vec_add_trunc_multipredict>(
          all, ec, mp, num_features_from_interactions);
    }
    else
    {
      VW::foreach_feature<VW::details::multipredict_info<VW::dense_parameters>, uint64_t,
          VW::details::vec_add_multipredict>(all, ec, mp, num_features_from_interactions);
    }
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

class power_data
{
public:
  float minus_power_t;
  float neg_norm_power;
};

template <bool sqrt_rate, size_t adaptive, size_t normalized>
inline float compute_rate_decay(power_data& s, float& fw)
{
  VW::weight* w = &fw;
  float rate_decay = 1.f;
  if (adaptive)
  {
    if (sqrt_rate) { rate_decay = inv_sqrt(w[adaptive]); }
    else { rate_decay = powf(w[adaptive], s.minus_power_t); }
  }
  if VW_STD17_CONSTEXPR (normalized != 0)
  {
    if (sqrt_rate)
    {
      float inv_norm = 1.f / w[normalized];
      if (adaptive) { rate_decay *= inv_norm; }
      else { rate_decay *= inv_norm * inv_norm; }
    }
    else { rate_decay *= powf(w[normalized] * w[normalized], s.neg_norm_power); }
  }
  return rate_decay;
}

class norm_data
{
public:
  float grad_squared;
  float pred_per_update;
  float norm_x;
  power_data pd;
  float extra_state[4];
  VW::io::logger* logger;
};

constexpr float X_MIN = 1.084202e-19f;
constexpr float X2_MIN = X_MIN * X_MIN;
constexpr float X2_MAX = FLT_MAX;

template <bool sqrt_rate, bool feature_mask_off, size_t adaptive, size_t normalized, size_t spare, bool stateless>
inline void pred_per_update_feature(norm_data& nd, float x, float& fw)
{
  bool modify = feature_mask_off || fw != 0.;
  if (modify)
  {
    VW::weight* w = &fw;
    float x2 = x * x;
    if (x2 < X2_MIN)
    {
      x = (x > 0) ? X_MIN : -X_MIN;
      x2 = X2_MIN;
    }
    if (stateless)  // we must not modify the parameter state so introduce a shadow version.
    {
      nd.extra_state[0] = w[0];
      nd.extra_state[adaptive] = w[adaptive];
      nd.extra_state[normalized] = w[normalized];
      w = nd.extra_state;
    }
    if (adaptive) { w[adaptive] += nd.grad_squared * x2; }
    if VW_STD17_CONSTEXPR (normalized != 0)
    {
      float x_abs = fabsf(x);
      if (x_abs > w[normalized])  // new scale discovered
      {
        if (w[normalized] >
            0.)  // If the normalizer is > 0 then rescale the weight so it's as if the new scale was the old scale.
        {
          if (sqrt_rate)
          {
            float rescale = w[normalized] / x_abs;
            w[0] *= (adaptive ? rescale : rescale * rescale);
          }
          else
          {
            float rescale = x_abs / w[normalized];
            w[0] *= powf(rescale * rescale, nd.pd.neg_norm_power);
          }
        }
        w[normalized] = x_abs;
      }
      float norm_x2 = x2 / (w[normalized] * w[normalized]);
      if (x2 > X2_MAX)
      {
        norm_x2 = 1;
        assert(nd.logger != nullptr);
        nd.logger->err_error("The features have too much magnitude");
      }
      nd.norm_x += norm_x2;
    }
    w[spare] = compute_rate_decay<sqrt_rate, adaptive, normalized>(nd.pd, w[0]);
    nd.pred_per_update += x2 * w[spare];
  }
}

template <bool sqrt_rate, bool feature_mask_off, bool adax, size_t adaptive, size_t normalized, size_t spare,
    bool stateless>
float get_pred_per_update(VW::reductions::gd& g, VW::example& ec)
{
  // We must traverse the features in _precisely_ the same order as during training.
  auto& ld = ec.l.simple;
  VW::workspace& all = *g.all;

  float grad_squared = ec.weight;
  if (!adax) { grad_squared *= all.loss->get_square_grad(ec.pred.scalar, ld.label); }

  if (grad_squared == 0 && !stateless) { return 1.; }

  norm_data nd = {grad_squared, 0., 0., {g.neg_power_t, g.neg_norm_power}, {0}, &g.all->logger};
  VW::foreach_feature<norm_data,
      pred_per_update_feature<sqrt_rate, feature_mask_off, adaptive, normalized, spare, stateless>>(all, ec, nd);
  if VW_STD17_CONSTEXPR (normalized != 0)
  {
    if (!stateless)
    {
      g.per_model_states[0].normalized_sum_norm_x += (static_cast<double>(ec.weight)) * nd.norm_x;
      g.per_model_states[0].total_weight += ec.weight;
      g.update_multiplier =
          average_update<sqrt_rate, adaptive, normalized>(static_cast<float>(g.per_model_states[0].total_weight),
              static_cast<float>(g.per_model_states[0].normalized_sum_norm_x), g.neg_norm_power);
    }
    else
    {
      float nsnx = (static_cast<float>(g.per_model_states[0].normalized_sum_norm_x)) + ec.weight * nd.norm_x;
      float tw = static_cast<float>(g.per_model_states[0].total_weight) + ec.weight;
      g.update_multiplier = average_update<sqrt_rate, adaptive, normalized>(tw, nsnx, g.neg_norm_power);
    }
    nd.pred_per_update *= g.update_multiplier;
  }
  return nd.pred_per_update;
}

template <bool sqrt_rate, bool feature_mask_off, bool adax, size_t adaptive, size_t normalized, size_t spare,
    bool stateless>
float sensitivity(VW::reductions::gd& g, VW::example& ec)
{
  if VW_STD17_CONSTEXPR (adaptive || normalized)
  {
    return get_pred_per_update<sqrt_rate, feature_mask_off, adax, adaptive, normalized, spare, stateless>(g, ec);
  }
  else
  {
    _UNUSED(g);
    return ec.get_total_sum_feat_sq();
  }
}
VW_WARNING_STATE_POP

template <size_t adaptive>
float get_scale(VW::reductions::gd& g, VW::example& /* ec */, float weight)
{
  float update_scale = g.all->eta * weight;
  if (!adaptive)
  {
    float t = static_cast<float>(
        g.all->sd->t + weight - g.all->sd->weighted_holdout_examples - g.all->sd->weighted_unlabeled_examples);
    update_scale *= powf(t, g.neg_power_t);
  }
  return update_scale;
}

template <bool sqrt_rate, bool feature_mask_off, bool adax, size_t adaptive, size_t normalized, size_t spare>
float sensitivity(VW::reductions::gd& g, VW::example& ec)
{
  return get_scale<adaptive>(g, ec, 1.) *
      sensitivity<sqrt_rate, feature_mask_off, adax, adaptive, normalized, spare, true>(g, ec);
}

template <bool sparse_l2, bool invariant, bool sqrt_rate, bool feature_mask_off, bool adax, size_t adaptive,
    size_t normalized, size_t spare>
float compute_update(VW::reductions::gd& g, VW::example& ec)
{
  // invariant: not a test label, importance weight > 0
  const auto& ld = ec.l.simple;
  VW::workspace& all = *g.all;

  float update = 0.;
  ec.updated_prediction = ec.pred.scalar;
  if (all.loss->get_loss(all.sd.get(), ec.pred.scalar, ld.label) > 0.)
  {
    float pred_per_update = sensitivity<sqrt_rate, feature_mask_off, adax, adaptive, normalized, spare, false>(g, ec);
    float update_scale = get_scale<adaptive>(g, ec, ec.weight);
    if (invariant) { update = all.loss->get_update(ec.pred.scalar, ld.label, update_scale, pred_per_update); }
    else { update = all.loss->get_unsafe_update(ec.pred.scalar, ld.label, update_scale); }
    // changed from ec.partial_prediction to ld.prediction
    ec.updated_prediction += pred_per_update * update;

    if (all.reg_mode && std::fabs(update) > 1e-8)
    {
      double dev1 = all.loss->first_derivative(all.sd.get(), ec.pred.scalar, ld.label);
      double eta_bar = (fabs(dev1) > 1e-8) ? (-update / dev1) : 0.0;
      if (fabs(dev1) > 1e-8) { all.sd->contraction *= (1. - all.l2_lambda * eta_bar); }
      update /= static_cast<float>(all.sd->contraction);
      all.sd->gravity += eta_bar * all.l1_lambda;
    }
  }

  if (sparse_l2) { update -= g.sparse_l2 * ec.pred.scalar; }

  if (std::isnan(update))
  {
    g.all->logger.err_warn("update is NAN, replacing with 0");
    update = 0.;
  }

  return update;
}

template <bool sparse_l2, bool invariant, bool sqrt_rate, bool feature_mask_off, bool adax, size_t adaptive,
    size_t normalized, size_t spare>
void update(VW::reductions::gd& g, VW::example& ec)
{
  // invariant: not a test label, importance weight > 0
  float update;
  if ((update = compute_update<sparse_l2, invariant, sqrt_rate, feature_mask_off, adax, adaptive, normalized, spare>(
           g, ec)) != 0.)
  {
    train<sqrt_rate, feature_mask_off, adaptive, normalized, spare>(g, ec, update);
  }

  if (g.all->sd->contraction < 1e-9 || g.all->sd->gravity > 1e3)
  {  // updating weights now to avoid numerical instability
    sync_weights(*g.all);
  }
}

template <bool sparse_l2, bool invariant, bool sqrt_rate, bool feature_mask_off, bool adax, size_t adaptive,
    size_t normalized, size_t spare>
void learn(VW::reductions::gd& g, VW::example& ec)
{
  // invariant: not a test label, importance weight > 0
  assert(ec.l.simple.label != FLT_MAX);
  assert(ec.weight > 0.);
  g.predict(g, ec);
  update<sparse_l2, invariant, sqrt_rate, feature_mask_off, adax, adaptive, normalized, spare>(g, ec);
}

size_t write_index(VW::io_buf& model_file, std::stringstream& msg, bool text, uint32_t num_bits, uint64_t i)
{
  size_t brw;
  uint32_t old_i = 0;

  msg << i;

  if (num_bits < 31)
  {
    old_i = static_cast<uint32_t>(i);
    brw = VW::details::bin_text_write_fixed(model_file, reinterpret_cast<char*>(&old_i), sizeof(old_i), msg, text);
  }
  else { brw = VW::details::bin_text_write_fixed(model_file, reinterpret_cast<char*>(&i), sizeof(i), msg, text); }

  return brw;
}

std::string to_string(const VW::details::invert_hash_info& info)
{
  std::ostringstream ss;
  for (size_t i = 0; i < info.weight_components.size(); i++)
  {
    if (i > 0) { ss << "*"; }
    ss << VW::to_string(info.weight_components[i]);
  }
  if (info.offset != 0)
  {
    // otherwise --oaa output no features for class > 0.
    ss << '[' << (info.offset >> info.stride_shift) << ']';
  }
  return ss.str();
}

template <class T>
void save_load_regressor(VW::workspace& all, VW::io_buf& model_file, bool read, bool text, T& weights)
{
  size_t brw = 1;

  if (all.print_invert)  // write readable model with feature names
  {
    std::stringstream msg;

    for (auto it = weights.begin(); it != weights.end(); ++it)
    {
      const auto weight_value = *it;
      if (*it != 0.f)
      {
        const auto weight_index = it.index() >> weights.stride_shift();

        const auto map_it = all.index_name_map.find(weight_index);
        if (map_it != all.index_name_map.end())
        {
          msg << to_string(map_it->second);
          VW::details::bin_text_write_fixed(model_file, nullptr /*unused*/, 0 /*unused*/, msg, true);
        }

        msg << ":" << weight_index << ":" << weight_value << "\n";
        VW::details::bin_text_write_fixed(model_file, nullptr /*unused*/, 0 /*unused*/, msg, true);
      }
    }
    return;
  }

  uint64_t i = 0;
  uint32_t old_i = 0;
  uint64_t length = static_cast<uint64_t>(1) << all.num_bits;
  if (read)
  {
    do {
      brw = 1;
      if (all.num_bits < 31)  // backwards compatible
      {
        brw = model_file.bin_read_fixed(reinterpret_cast<char*>(&old_i), sizeof(old_i));
        i = old_i;
      }
      else { brw = model_file.bin_read_fixed(reinterpret_cast<char*>(&i), sizeof(i)); }
      if (brw > 0)
      {
        if (i >= length)
          THROW("Model content is corrupted, weight vector index " << i << " must be less than total vector length "
                                                                   << length);
        VW::weight* v = &weights.strided_index(i);
        brw += model_file.bin_read_fixed(reinterpret_cast<char*>(&(*v)), sizeof(*v));
      }
    } while (brw > 0);
  }
  else  // write
  {
    for (typename T::iterator v = weights.begin(); v != weights.end(); ++v)
    {
      if (*v != 0.)
      {
        i = v.index() >> weights.stride_shift();
        std::stringstream msg;
        brw = write_index(model_file, msg, text, all.num_bits, i);
        msg << ":" << *v << "\n";
        brw += VW::details::bin_text_write_fixed(model_file, (char*)&(*v), sizeof(*v), msg, text);
      }
    }
  }
}
}  // namespace

void VW::details::save_load_regressor_gd(VW::workspace& all, VW::io_buf& model_file, bool read, bool text)
{
  if (all.weights.sparse) { ::save_load_regressor(all, model_file, read, text, all.weights.sparse_weights); }
  else { ::save_load_regressor(all, model_file, read, text, all.weights.dense_weights); }
}

namespace
{
template <class T>
void save_load_online_state_weights(VW::workspace& all, VW::io_buf& model_file, bool read, bool text,
    VW::reductions::gd* g, std::stringstream& msg, uint32_t ftrl_size, T& weights)
{
  uint64_t length = static_cast<uint64_t>(1) << all.num_bits;

  uint64_t i = 0;
  uint32_t old_i = 0;
  size_t brw = 1;

  if (read)
  {
    do {
      brw = 1;
      if (all.num_bits < 31)  // backwards compatible
      {
        brw = model_file.bin_read_fixed(reinterpret_cast<char*>(&old_i), sizeof(old_i));
        i = old_i;
      }
      else { brw = model_file.bin_read_fixed(reinterpret_cast<char*>(&i), sizeof(i)); }
      if (brw > 0)
      {
        if (i >= length)
          THROW("Model content is corrupted, weight vector index " << i << " must be less than total vector length "
                                                                   << length);
        VW::weight buff[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        if (ftrl_size > 0)
        {
          brw += model_file.bin_read_fixed(reinterpret_cast<char*>(buff), sizeof(buff[0]) * ftrl_size);
        }
        else if (g == nullptr || (!g->adaptive_input && !g->normalized_input))
        {
          brw += model_file.bin_read_fixed(reinterpret_cast<char*>(buff), sizeof(buff[0]));
        }
        else if ((g->adaptive_input && !g->normalized_input) || (!g->adaptive_input && g->normalized_input))
        {
          brw += model_file.bin_read_fixed(reinterpret_cast<char*>(buff), sizeof(buff[0]) * 2);
        }
        else
        {  // adaptive and normalized
          brw += model_file.bin_read_fixed(reinterpret_cast<char*>(buff), sizeof(buff[0]) * 3);
        }
        uint32_t stride = 1 << weights.stride_shift();
        VW::weight* v = &weights.strided_index(i);
        for (size_t j = 0; j < stride; j++) { v[j] = buff[j]; }
      }
    } while (brw > 0);
  }
  else
  {  // write binary or text
    if (all.hexfloat_weights && (text || all.print_invert)) { msg << std::hexfloat; }

    for (typename T::iterator v = weights.begin(); v != weights.end(); ++v)
    {
      i = v.index() >> weights.stride_shift();
      bool gd_write = *v != 0.f;
      bool ftrl3_write = ftrl_size == 3 && (*v != 0.f || (&(*v))[1] != 0.f || (&(*v))[2] != 0.f);
      bool ftrl4_write = ftrl_size == 4 && (*v != 0.f || (&(*v))[1] != 0.f || (&(*v))[2] != 0.f || (&(*v))[3] != 0.f);
      bool ftrl6_write = ftrl_size == 6 &&
          (*v != 0.f || (&(*v))[1] != 0.f || (&(*v))[2] != 0.f || (&(*v))[3] != 0.f || (&(*v))[4] != 0.f ||
              (&(*v))[5] != 0.f);
      if (all.print_invert)  // write readable model with feature names
      {
        if (gd_write || ftrl3_write || ftrl4_write || ftrl6_write)
        {
          const auto map_it = all.index_name_map.find(i);
          if (map_it != all.index_name_map.end()) { msg << to_string(map_it->second) << ":"; }
        }
      }

      if (ftrl3_write)
      {
        brw = write_index(model_file, msg, text, all.num_bits, i);
        msg << ":" << *v << " " << (&(*v))[1] << " " << (&(*v))[2] << "\n";
        brw += VW::details::bin_text_write_fixed(model_file, (char*)&(*v), 3 * sizeof(*v), msg, text);
      }
      else if (ftrl4_write)
      {
        brw = write_index(model_file, msg, text, all.num_bits, i);
        msg << ":" << *v << " " << (&(*v))[1] << " " << (&(*v))[2] << " " << (&(*v))[3] << "\n";
        brw += VW::details::bin_text_write_fixed(model_file, (char*)&(*v), 4 * sizeof(*v), msg, text);
      }
      else if (ftrl6_write)
      {
        brw = write_index(model_file, msg, text, all.num_bits, i);
        msg << ":" << *v << " " << (&(*v))[1] << " " << (&(*v))[2] << " " << (&(*v))[3] << " " << (&(*v))[4] << " "
            << (&(*v))[5] << "\n";
        brw += VW::details::bin_text_write_fixed(model_file, (char*)&(*v), 6 * sizeof(*v), msg, text);
      }
      else if (g == nullptr || (!all.weights.adaptive && !all.weights.normalized))
      {
        if (*v != 0.)
        {
          brw = write_index(model_file, msg, text, all.num_bits, i);
          msg << ":" << *v << "\n";
          brw += VW::details::bin_text_write_fixed(model_file, (char*)&(*v), sizeof(*v), msg, text);
        }
      }
      else if ((all.weights.adaptive && !all.weights.normalized) || (!all.weights.adaptive && all.weights.normalized))
      {
        // either adaptive or normalized
        if (*v != 0. || (&(*v))[1] != 0.)
        {
          brw = write_index(model_file, msg, text, all.num_bits, i);
          msg << ":" << *v << " " << (&(*v))[1] << "\n";
          brw += VW::details::bin_text_write_fixed(model_file, (char*)&(*v), 2 * sizeof(*v), msg, text);
        }
      }
      else
      {
        // adaptive and normalized
        if (*v != 0. || (&(*v))[1] != 0. || (&(*v))[2] != 0.)
        {
          brw = write_index(model_file, msg, text, all.num_bits, i);
          msg << ":" << *v << " " << (&(*v))[1] << " " << (&(*v))[2] << "\n";
          brw += VW::details::bin_text_write_fixed(model_file, (char*)&(*v), 3 * sizeof(*v), msg, text);
        }
      }
    }
  }
}
}  // namespace

void VW::details::save_load_online_state_gd(VW::workspace& all, VW::io_buf& model_file, bool read, bool text,
    double& total_weight, double& normalized_sum_norm_x, VW::reductions::gd* g, uint32_t ftrl_size)
{
  std::stringstream msg;

  msg << "initial_t " << all.initial_t << "\n";
  VW::details::bin_text_read_write_fixed(
      model_file, reinterpret_cast<char*>(&all.initial_t), sizeof(all.initial_t), read, msg, text);

  msg << "norm normalizer " << normalized_sum_norm_x << "\n";
  VW::details::bin_text_read_write_fixed(
      model_file, reinterpret_cast<char*>(&normalized_sum_norm_x), sizeof(normalized_sum_norm_x), read, msg, text);

  msg << "t " << all.sd->t << "\n";
  VW::details::bin_text_read_write_fixed(
      model_file, reinterpret_cast<char*>(&all.sd->t), sizeof(all.sd->t), read, msg, text);

  msg << "sum_loss " << all.sd->sum_loss << "\n";
  VW::details::bin_text_read_write_fixed(
      model_file, reinterpret_cast<char*>(&all.sd->sum_loss), sizeof(all.sd->sum_loss), read, msg, text);

  msg << "sum_loss_since_last_dump " << all.sd->sum_loss_since_last_dump << "\n";
  VW::details::bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&all.sd->sum_loss_since_last_dump),
      sizeof(all.sd->sum_loss_since_last_dump), read, msg, text);

  float dump_interval = all.sd->dump_interval;
  msg << "dump_interval " << dump_interval << "\n";
  VW::details::bin_text_read_write_fixed(
      model_file, reinterpret_cast<char*>(&dump_interval), sizeof(dump_interval), read, msg, text);
  if (!read || (all.training && all.preserve_performance_counters))
  {  // update dump_interval from input model
    all.sd->dump_interval = dump_interval;
  }

  msg << "min_label " << all.sd->min_label << "\n";
  VW::details::bin_text_read_write_fixed(
      model_file, reinterpret_cast<char*>(&all.sd->min_label), sizeof(all.sd->min_label), read, msg, text);

  msg << "max_label " << all.sd->max_label << "\n";
  VW::details::bin_text_read_write_fixed(
      model_file, reinterpret_cast<char*>(&all.sd->max_label), sizeof(all.sd->max_label), read, msg, text);

  msg << "weighted_labeled_examples " << all.sd->weighted_labeled_examples << "\n";
  VW::details::bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&all.sd->weighted_labeled_examples),
      sizeof(all.sd->weighted_labeled_examples), read, msg, text);

  msg << "weighted_labels " << all.sd->weighted_labels << "\n";
  VW::details::bin_text_read_write_fixed(
      model_file, reinterpret_cast<char*>(&all.sd->weighted_labels), sizeof(all.sd->weighted_labels), read, msg, text);

  msg << "weighted_unlabeled_examples " << all.sd->weighted_unlabeled_examples << "\n";
  VW::details::bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&all.sd->weighted_unlabeled_examples),
      sizeof(all.sd->weighted_unlabeled_examples), read, msg, text);

  msg << "example_number " << all.sd->example_number << "\n";
  VW::details::bin_text_read_write_fixed(
      model_file, reinterpret_cast<char*>(&all.sd->example_number), sizeof(all.sd->example_number), read, msg, text);

  msg << "total_features " << all.sd->total_features << "\n";
  VW::details::bin_text_read_write_fixed(
      model_file, reinterpret_cast<char*>(&all.sd->total_features), sizeof(all.sd->total_features), read, msg, text);

  if (!read || all.model_file_ver >= VW::version_definitions::VERSION_SAVE_RESUME_FIX)
  {
    // restore some data to allow save_resume work more accurate

    // fix average loss
    msg << "total_weight " << total_weight << "\n";
    VW::details::bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&total_weight), sizeof(total_weight), read, msg, text);

    // fix "loss since last" for first printed out example details
    msg << "sd::oec.weighted_labeled_examples " << all.sd->old_weighted_labeled_examples << "\n";
    VW::details::bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&all.sd->old_weighted_labeled_examples),
        sizeof(all.sd->old_weighted_labeled_examples), read, msg, text);

    // fix "number of examples per pass"
    msg << "current_pass " << all.current_pass << "\n";
    if (all.model_file_ver >= VW::version_definitions::VERSION_PASS_UINT64)
    {
      VW::details::bin_text_read_write_fixed(
          model_file, reinterpret_cast<char*>(&all.current_pass), sizeof(all.current_pass), read, msg, text);
    }
    else  // backwards compatiblity.
    {
      size_t temp_pass = static_cast<size_t>(all.current_pass);
      VW::details::bin_text_read_write_fixed(
          model_file, reinterpret_cast<char*>(&temp_pass), sizeof(temp_pass), read, msg, text);
      all.current_pass = temp_pass;
    }
  }

  if (!read || all.model_file_ver >= VW::version_definitions::VERSION_FILE_WITH_L1_AND_L2_STATE_IN_MODEL_DATA)
  {
    msg << "l1_state " << all.sd->gravity << "\n";
    auto local_gravity = all.sd->gravity;
    VW::details::bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&local_gravity), sizeof(local_gravity), read, msg, text);

    // all.sd->gravity - command line value
    // local_gravity - model value
    // 1. If command line value is non-default, use that value
    // 2. Else if model is non-default, use that value
    // 3. Else use default
    if (read && (all.sd->gravity == L1_STATE_DEFAULT) && (local_gravity != L1_STATE_DEFAULT))
    {
      all.sd->gravity = local_gravity;
    }

    msg << "l2_state " << all.sd->contraction << "\n";
    auto local_contraction = all.sd->contraction;
    VW::details::bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&local_contraction), sizeof(local_contraction), read, msg, text);

    // all.sd->contraction - command line value
    // local_contraction - model value
    // 1. If command line value is non-default, use that value
    // 2. Else if model is non-default, use that value
    // 3. Else use default
    if (read && (all.sd->contraction == L2_STATE_DEFAULT) && (local_contraction != L2_STATE_DEFAULT))
    {
      all.sd->contraction = local_contraction;
    }
  }

  if (read &&
      (!all.training ||
          !all.preserve_performance_counters))  // reset various things so that we report test set performance properly
  {
    all.sd->sum_loss = 0;
    all.sd->sum_loss_since_last_dump = 0;
    all.sd->weighted_labeled_examples = 0.;
    all.sd->weighted_labels = 0.;
    all.sd->weighted_unlabeled_examples = 0.;
    all.sd->old_weighted_labeled_examples = 0.;
    all.sd->example_number = 0;
    all.sd->total_features = 0;
    all.current_pass = 0;
  }
  if (all.weights.sparse)
  {
    save_load_online_state_weights(all, model_file, read, text, g, msg, ftrl_size, all.weights.sparse_weights);
  }
  else { save_load_online_state_weights(all, model_file, read, text, g, msg, ftrl_size, all.weights.dense_weights); }
}

namespace
{
void save_load(VW::reductions::gd& g, VW::io_buf& model_file, bool read, bool text)
{
  VW::workspace& all = *g.all;
  if (read)
  {
    VW::details::initialize_regressor(all);

    if (all.weights.adaptive && all.initial_t > 0)
    {
      float init_weight = all.initial_weight;
      float init_t = all.initial_t;
      auto initial_gd_weight_initializer = [init_weight, init_t](VW::weight* weights, uint64_t /*index*/)
      {
        weights[0] = init_weight;
        weights[1] = init_t;
      };

      all.weights.set_default(initial_gd_weight_initializer);

      // for adaptive update, we interpret initial_t as previously seeing initial_t fake datapoints, all with squared
      // gradient=1 NOTE: this is not invariant to the scaling of the data (i.e. when combined with normalized). Since
      // scaling the data scales the gradient, this should ideally be feature_range*initial_t, or something like that.
      // We could potentially fix this by just adding this base quantity times the current range to the sum of gradients
      // stored in memory at each update, and always start sum of gradients to 0, at the price of additional additions
      // and multiplications during the update...
    }
    if (g.initial_constant != 0.0) { VW::set_weight(all, VW::details::CONSTANT, 0, g.initial_constant); }
  }

  if (model_file.num_files() > 0)
  {
    bool resume = all.save_resume;
    std::stringstream msg;
    msg << ":" << resume << "\n";
    VW::details::bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&resume), sizeof(resume), read, msg, text);
    if (resume)
    {
      if (read && all.model_file_ver < VW::version_definitions::VERSION_SAVE_RESUME_FIX)
      {
        g.all->logger.err_warn(
            "save_resume functionality is known to have inaccuracy in model files version less than '{}'",
            VW::version_definitions::VERSION_SAVE_RESUME_FIX.to_string());
      }
      VW::details::save_load_online_state_gd(all, model_file, read, text, g.per_model_states[0].total_weight,
          g.per_model_states[0].normalized_sum_norm_x, &g);
    }
    else
    {
      if (!all.weights.not_null()) { THROW("Model weights not initialized."); }
      VW::details::save_load_regressor_gd(all, model_file, read, text);
    }
  }
  if (!all.training)
  {  // If the regressor was saved without --predict_only_model, then when testing we want to
     // materialize the weights.
    sync_weights(all);
  }
}

template <bool sparse_l2, bool invariant, bool sqrt_rate, bool feature_mask_off, uint64_t adaptive, uint64_t normalized,
    uint64_t spare, uint64_t next>
uint64_t set_learn(VW::workspace& all, VW::reductions::gd& g)
{
  all.normalized_idx = normalized;
  if (g.adax)
  {
    g.learn = learn<sparse_l2, invariant, sqrt_rate, feature_mask_off, true, adaptive, normalized, spare>;
    g.update = update<sparse_l2, invariant, sqrt_rate, feature_mask_off, true, adaptive, normalized, spare>;
    g.sensitivity = sensitivity<sqrt_rate, feature_mask_off, true, adaptive, normalized, spare>;
    return next;
  }
  else
  {
    g.learn = learn<sparse_l2, invariant, sqrt_rate, feature_mask_off, false, adaptive, normalized, spare>;
    g.update = update<sparse_l2, invariant, sqrt_rate, feature_mask_off, false, adaptive, normalized, spare>;
    g.sensitivity = sensitivity<sqrt_rate, feature_mask_off, false, adaptive, normalized, spare>;
    return next;
  }
}

template <bool sparse_l2, bool invariant, bool sqrt_rate, uint64_t adaptive, uint64_t normalized, uint64_t spare,
    uint64_t next>
uint64_t set_learn(VW::workspace& all, bool feature_mask_off, VW::reductions::gd& g)
{
  all.normalized_idx = normalized;
  if (feature_mask_off)
  {
    return set_learn<sparse_l2, invariant, sqrt_rate, true, adaptive, normalized, spare, next>(all, g);
  }
  else { return set_learn<sparse_l2, invariant, sqrt_rate, false, adaptive, normalized, spare, next>(all, g); }
}

template <bool invariant, bool sqrt_rate, uint64_t adaptive, uint64_t normalized, uint64_t spare, uint64_t next>
uint64_t set_learn(VW::workspace& all, bool feature_mask_off, VW::reductions::gd& g)
{
  if (g.sparse_l2 > 0.f)
  {
    return set_learn<true, invariant, sqrt_rate, adaptive, normalized, spare, next>(all, feature_mask_off, g);
  }
  else { return set_learn<false, invariant, sqrt_rate, adaptive, normalized, spare, next>(all, feature_mask_off, g); }
}

template <bool sqrt_rate, uint64_t adaptive, uint64_t normalized, uint64_t spare, uint64_t next>
uint64_t set_learn(VW::workspace& all, bool feature_mask_off, VW::reductions::gd& g)
{
  if (all.invariant_updates)
  {
    return set_learn<true, sqrt_rate, adaptive, normalized, spare, next>(all, feature_mask_off, g);
  }
  else { return set_learn<false, sqrt_rate, adaptive, normalized, spare, next>(all, feature_mask_off, g); }
}

template <bool sqrt_rate, uint64_t adaptive, uint64_t spare>
uint64_t set_learn(VW::workspace& all, bool feature_mask_off, VW::reductions::gd& g)
{
  // select the appropriate learn function based on adaptive, normalization, and feature mask
  if (all.weights.normalized)
  {
    return set_learn<sqrt_rate, adaptive, adaptive + 1, adaptive + 2, adaptive + 3>(all, feature_mask_off, g);
  }
  else { return set_learn<sqrt_rate, adaptive, 0, spare, spare + 1>(all, feature_mask_off, g); }
}

template <bool sqrt_rate>
uint64_t set_learn(VW::workspace& all, bool feature_mask_off, VW::reductions::gd& g)
{
  if (all.weights.adaptive) { return set_learn<sqrt_rate, 1, 2>(all, feature_mask_off, g); }
  else { return set_learn<sqrt_rate, 0, 0>(all, feature_mask_off, g); }
}

uint64_t ceil_log_2(uint64_t v)
{
  if (v == 0) { return 0; }
  else { return 1 + ceil_log_2(v >> 1); }
}

}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::gd_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();

  auto g = VW::make_unique<VW::reductions::gd>();

  bool sgd = false;
  bool adaptive = false;
  bool adax = false;
  bool invariant = false;
  bool normalized = false;

  all.sd->gravity = L1_STATE_DEFAULT;
  all.sd->contraction = L2_STATE_DEFAULT;
  float local_gravity = 0;
  float local_contraction = 0;

  option_group_definition new_options("[Reduction] Gradient Descent");
  new_options.add(make_option("sgd", sgd).help("Use regular stochastic gradient descent update").keep(all.save_resume))
      .add(make_option("adaptive", adaptive).help("Use adaptive, individual learning rates").keep(all.save_resume))
      .add(make_option("adax", adax).help("Use adaptive learning rates with x^2 instead of g^2x^2"))
      .add(make_option("invariant", invariant).help("Use safe/importance aware updates").keep(all.save_resume))
      .add(make_option("normalized", normalized).help("Use per feature normalized updates").keep(all.save_resume))
      .add(make_option("sparse_l2", g->sparse_l2)
               .default_value(0.f)
               .help("Degree of l2 regularization applied to activated sparse parameters"))
      .add(make_option("l1_state", local_gravity)
               .allow_override()
               .default_value(L1_STATE_DEFAULT)
               .help("Amount of accumulated implicit l1 regularization"))
      .add(make_option("l2_state", local_contraction)
               .allow_override()
               .default_value(L2_STATE_DEFAULT)
               .help("Amount of accumulated implicit l2 regularization"));
  options.add_and_parse(new_options);

  if (options.was_supplied("l1_state")) { all.sd->gravity = local_gravity; }
  if (options.was_supplied("l2_state")) { all.sd->contraction = local_contraction; }

  g->all = &all;
  auto single_model_state = details::per_model_state();
  single_model_state.normalized_sum_norm_x = 0;
  single_model_state.total_weight = 0.;
  g->per_model_states.emplace_back(single_model_state);
  g->no_win_counter = 0;
  all.weights.adaptive = true;
  all.weights.normalized = true;
  g->neg_norm_power = (all.weights.adaptive ? (all.power_t - 1.f) : -1.f);
  g->neg_power_t = -all.power_t;

  if (all.initial_t > 0)  // for the normalized update: if initial_t is bigger than 1 we interpret this as if we had
                          // seen (all.initial_t) previous fake datapoints all with norm 1
  {
    g->per_model_states[0].normalized_sum_norm_x = all.initial_t;
    g->per_model_states[0].total_weight = all.initial_t;
  }

  bool feature_mask_off = true;
  if (options.was_supplied("feature_mask")) { feature_mask_off = false; }

  if (!all.holdout_set_off)
  {
    all.sd->holdout_best_loss = FLT_MAX;
    g->early_stop_thres = options.get_typed_option<uint64_t>("early_terminate").value();
  }

  g->initial_constant = all.initial_constant;

  if (sgd || adaptive || invariant || normalized)
  {
    // nondefault
    all.weights.adaptive = adaptive;
    all.invariant_updates = all.training && invariant;
    all.weights.normalized = normalized;

    if (!options.was_supplied("learning_rate") && !options.was_supplied("l") &&
        !(all.weights.adaptive && all.weights.normalized))
    {
      all.eta = 10;  // default learning rate to 10 for non default update rule
    }

    // if not using normalized or adaptive, default initial_t to 1 instead of 0
    if (!all.weights.adaptive && !all.weights.normalized)
    {
      if (!options.was_supplied("initial_t"))
      {
        all.sd->t = 1.f;
        all.initial_t = 1.f;
      }
      all.eta *= powf(static_cast<float>(all.sd->t), all.power_t);
    }
  }
  else { all.invariant_updates = all.training; }
  g->adaptive_input = all.weights.adaptive;
  g->normalized_input = all.weights.normalized;

  all.weights.adaptive = all.weights.adaptive && all.training;
  all.weights.normalized = all.weights.normalized && all.training;

  if (adax) { g->adax = all.training && adax; }

  if (g->adax && !all.weights.adaptive) THROW("Cannot use adax without adaptive");

  if (pow(static_cast<double>(all.eta_decay_rate), static_cast<double>(all.numpasses)) < 0.0001)
  {
    all.logger.err_warn(
        "The learning rate for the last pass is multiplied by '{}' adjust --decay_learning_rate larger to avoid this.",
        pow(static_cast<double>(all.eta_decay_rate), static_cast<double>(all.numpasses)));
  }

  if (all.reg_mode % 2)
  {
    if (all.audit || all.hash_inv)
    {
      g->predict = ::predict<true, true>;
      g->multipredict = ::multipredict<true, true>;
    }
    else
    {
      g->predict = ::predict<true, false>;
      g->multipredict = ::multipredict<true, false>;
    }
  }
  else if (all.audit || all.hash_inv)
  {
    g->predict = ::predict<false, true>;
    g->multipredict = ::multipredict<false, true>;
  }
  else
  {
    g->predict = ::predict<false, false>;
    g->multipredict = ::multipredict<false, false>;
  }

  uint64_t stride;
  if (all.power_t == 0.5) { stride = ::set_learn<true>(all, feature_mask_off, *g.get()); }
  else { stride = ::set_learn<false>(all, feature_mask_off, *g.get()); }

  all.weights.stride_shift(static_cast<uint32_t>(::ceil_log_2(stride - 1)));

  auto* bare = g.get();
  auto l = make_bottom_learner(std::move(g), g->learn, bare->predict, stack_builder.get_setupfn_name(gd_setup),
      VW::prediction_type_t::SCALAR, VW::label_type_t::SIMPLE)
               .set_learn_returns_prediction(true)
               .set_params_per_weight(VW::details::UINT64_ONE << all.weights.stride_shift())
               .set_sensitivity(bare->sensitivity)
               .set_multipredict(bare->multipredict)
               .set_update(bare->update)
               .set_save_load(::save_load)
               .set_end_pass(::end_pass)
               .set_merge_with_all(::merge)
               .set_add_with_all(::add)
               .set_subtract_with_all(::subtract)
               .set_output_example_prediction(VW::details::output_example_prediction_simple_label<VW::reductions::gd>)
               .set_update_stats(VW::details::update_stats_simple_label<VW::reductions::gd>)
               .set_print_update(VW::details::print_update_simple_label<VW::reductions::gd>)
               .build();
  return l;
}
