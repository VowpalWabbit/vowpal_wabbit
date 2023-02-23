// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/array_parameters.h"
#include "vw/core/constant.h"
#include "vw/core/example.h"
#include "vw/core/gd_predict.h"
#include "vw/core/global_data.h"
#include "vw/core/interactions.h"
#include "vw/core/vw_math.h"

// we need it for learner
#include "vw/core/vw_fwd.h"

#include <memory>

namespace VW
{
namespace reductions
{
std::shared_ptr<VW::LEARNER::learner> gd_setup(VW::setup_base_i& stack_builder);

namespace details
{

class per_model_state
{
public:
  double normalized_sum_norm_x = 0.0;
  double total_weight = 0.0;
};
}  // namespace details

class gd
{
public:
  std::vector<details::per_model_state> per_model_states;
  size_t no_win_counter = 0;
  size_t early_stop_thres = 0;
  float initial_constant = 0.f;
  float neg_norm_power = 0.f;
  float neg_power_t = 0.f;
  float sparse_l2 = 0.f;
  float update_multiplier = 0.f;
  void (*predict)(gd&, VW::example&) = nullptr;
  void (*learn)(gd&, VW::example&) = nullptr;
  void (*update)(gd&, VW::example&) = nullptr;
  float (*sensitivity)(gd&, VW::example&) = nullptr;
  void (*multipredict)(gd&, VW::example&, size_t, size_t, VW::polyprediction*, bool) = nullptr;
  bool adaptive_input = false;
  bool normalized_input = false;
  bool adax = false;
  VW::workspace* all = nullptr;  // parallel, features, parameters
};
}  // namespace reductions

namespace details
{

float finalize_prediction(VW::shared_data& sd, VW::io::logger& logger, float ret);
void print_features(VW::workspace& all, VW::example& ec);
void print_audit_features(VW::workspace&, VW::example& ec);
void save_load_regressor_gd(VW::workspace& all, VW::io_buf& model_file, bool read, bool text);
void save_load_online_state_gd(VW::workspace& all, VW::io_buf& model_file, bool read, bool text, double& total_weight,
    double& normalized_sum_norm_x, VW::reductions::gd* g = nullptr, uint32_t ftrl_size = 0);

template <class T>
class multipredict_info
{
public:
  size_t count;
  size_t step;
  VW::polyprediction* pred;
  const T& weights; /* & for l1: */
  float gravity;
};

template <class T>
inline void vec_add_multipredict(multipredict_info<T>& mp, const float fx, uint64_t fi)
{
  if ((-1e-10 < fx) && (fx < 1e-10)) { return; }
  uint64_t mask = mp.weights.mask();
  VW::polyprediction* p = mp.pred;
  fi &= mask;
  uint64_t top = fi + (uint64_t)((mp.count - 1) * mp.step);
  uint64_t i = 0;
  if (top <= mask)
  {
    i += fi;
    for (; i <= top; i += mp.step, ++p)
    {
      p->scalar += fx * mp.weights[i];  // TODO: figure out how to use
                                        // weight_parameters::iterator (not using
                                        // change_begin())
    }
  }
  else
  {  // TODO: this could be faster by unrolling into two loops
    for (size_t c = 0; c < mp.count; ++c, fi += (uint64_t)mp.step, ++p)
    {
      fi &= mask;
      p->scalar += fx * mp.weights[fi];
    }
  }
}
}  // namespace details

// iterate through one namespace (or its part), callback function FuncT(some_data_R, feature_value_x, feature_weight)
template <class DataT, class WeightOrIndexT, void (*FuncT)(DataT&, float, WeightOrIndexT)>
inline void foreach_feature(VW::workspace& all, VW::example& ec, DataT& dat)
{
  return all.weights.sparse
      ? foreach_feature<DataT, WeightOrIndexT, FuncT, VW::sparse_parameters>(all.weights.sparse_weights,
            all.ignore_some_linear, all.ignore_linear, *ec.interactions, *ec.extent_interactions, all.permutations, ec,
            dat, all.generate_interactions_object_cache_state)
      : foreach_feature<DataT, WeightOrIndexT, FuncT, VW::dense_parameters>(all.weights.dense_weights,
            all.ignore_some_linear, all.ignore_linear, *ec.interactions, *ec.extent_interactions, all.permutations, ec,
            dat, all.generate_interactions_object_cache_state);
}

// iterate through one namespace (or its part), callback function FuncT(some_data_R, feature_value_x, feature_weight)
template <class DataT, class WeightOrIndexT, void (*FuncT)(DataT&, float, WeightOrIndexT)>
inline void foreach_feature(VW::workspace& all, VW::example& ec, DataT& dat, size_t& num_interacted_features)
{
  return all.weights.sparse
      ? foreach_feature<DataT, WeightOrIndexT, FuncT, VW::sparse_parameters>(all.weights.sparse_weights,
            all.ignore_some_linear, all.ignore_linear, *ec.interactions, *ec.extent_interactions, all.permutations, ec,
            dat, num_interacted_features, all.generate_interactions_object_cache_state)
      : foreach_feature<DataT, WeightOrIndexT, FuncT, VW::dense_parameters>(all.weights.dense_weights,
            all.ignore_some_linear, all.ignore_linear, *ec.interactions, *ec.extent_interactions, all.permutations, ec,
            dat, num_interacted_features, all.generate_interactions_object_cache_state);
}

// iterate through all namespaces and quadratic&cubic features, callback function T(some_data_R, feature_value_x,
// feature_weight)
template <class DataT, void (*FuncT)(DataT&, float, float&)>
inline void foreach_feature(VW::workspace& all, VW::example& ec, DataT& dat)
{
  foreach_feature<DataT, float&, FuncT>(all, ec, dat);
}

template <class DataT, void (*FuncT)(DataT&, float, float)>
inline void foreach_feature(VW::workspace& all, VW::example& ec, DataT& dat)
{
  foreach_feature<DataT, float, FuncT>(all, ec, dat);
}

template <class DataT, void (*FuncT)(DataT&, float, float&)>
inline void foreach_feature(VW::workspace& all, VW::example& ec, DataT& dat, size_t& num_interacted_features)
{
  foreach_feature<DataT, float&, FuncT>(all, ec, dat, num_interacted_features);
}

template <class DataT, void (*FuncT)(DataT&, float, const float&)>
inline void foreach_feature(VW::workspace& all, VW::example& ec, DataT& dat, size_t& num_interacted_features)
{
  foreach_feature<DataT, const float&, FuncT>(all, ec, dat, num_interacted_features);
}

inline float inline_predict(VW::workspace& all, VW::example& ec)
{
  const auto& simple_red_features = ec.ex_reduction_features.template get<VW::simple_label_reduction_features>();
  return all.weights.sparse ? inline_predict<VW::sparse_parameters>(all.weights.sparse_weights, all.ignore_some_linear,
                                  all.ignore_linear, *ec.interactions, *ec.extent_interactions, all.permutations, ec,
                                  all.generate_interactions_object_cache_state, simple_red_features.initial)
                            : inline_predict<VW::dense_parameters>(all.weights.dense_weights, all.ignore_some_linear,
                                  all.ignore_linear, *ec.interactions, *ec.extent_interactions, all.permutations, ec,
                                  all.generate_interactions_object_cache_state, simple_red_features.initial);
}

inline float inline_predict(VW::workspace& all, VW::example& ec, size_t& num_generated_features)
{
  const auto& simple_red_features = ec.ex_reduction_features.template get<VW::simple_label_reduction_features>();
  return all.weights.sparse
      ? inline_predict<VW::sparse_parameters>(all.weights.sparse_weights, all.ignore_some_linear, all.ignore_linear,
            *ec.interactions, *ec.extent_interactions, all.permutations, ec, num_generated_features,
            all.generate_interactions_object_cache_state, simple_red_features.initial)
      : inline_predict<VW::dense_parameters>(all.weights.dense_weights, all.ignore_some_linear, all.ignore_linear,
            *ec.interactions, *ec.extent_interactions, all.permutations, ec, num_generated_features,
            all.generate_interactions_object_cache_state, simple_red_features.initial);
}

inline float trunc_weight(const float w, const float gravity)
{
  return (gravity < fabsf(w)) ? w - VW::math::sign(w) * gravity : 0.f;
}

}  // namespace VW

namespace VW
{
template <class R, class S, void (*T)(R&, float, S), bool audit, void (*audit_func)(R&, const VW::audit_strings*)>
inline void generate_interactions(VW::workspace& all, VW::example_predict& ec, R& dat, size_t& num_interacted_features)
{
  if (all.weights.sparse)
  {
    generate_interactions<R, S, T, audit, audit_func, VW::sparse_parameters>(*ec.interactions, *ec.extent_interactions,
        all.permutations, ec, dat, all.weights.sparse_weights, num_interacted_features,
        all.generate_interactions_object_cache_state);
  }
  else
  {
    generate_interactions<R, S, T, audit, audit_func, VW::dense_parameters>(*ec.interactions, *ec.extent_interactions,
        all.permutations, ec, dat, all.weights.dense_weights, num_interacted_features,
        all.generate_interactions_object_cache_state);
  }
}

// this code is for C++98/03 complience as I unable to pass null function-pointer as template argument in g++-4.6
template <class R, class S, void (*T)(R&, float, S)>
inline void generate_interactions(VW::workspace& all, VW::example_predict& ec, R& dat, size_t& num_interacted_features)
{
  if (all.weights.sparse)
  {
    generate_interactions<R, S, T, VW::sparse_parameters>(all.interactions, all.extent_interactions, all.permutations,
        ec, dat, all.weights.sparse_weights, num_interacted_features, all.generate_interactions_object_cache_state);
  }
  else
  {
    generate_interactions<R, S, T, VW::dense_parameters>(all.interactions, all.extent_interactions, all.permutations,
        ec, dat, all.weights.dense_weights, num_interacted_features, all.generate_interactions_object_cache_state);
  }
}

}  // namespace VW

namespace INTERACTIONS  // NOLINT
{
template <class R, class S, void (*T)(R&, float, S), bool audit, void (*audit_func)(R&, const VW::audit_strings*)>
VW_DEPRECATED("Moved to VW namespace")
inline void generate_interactions(VW::workspace& all, VW::example_predict& ec, R& dat, size_t& num_interacted_features)
{
  // call version in VW namespace
  VW::generate_interactions<R, S, T, audit, audit_func>(all, ec, dat, num_interacted_features);
}

// this code is for C++98/03 complience as I unable to pass null function-pointer as template argument in g++-4.6
template <class R, class S, void (*T)(R&, float, S)>
VW_DEPRECATED("Moved to VW namespace")
inline void generate_interactions(VW::workspace& all, VW::example_predict& ec, R& dat, size_t& num_interacted_features)
{
  // call version in VW namespace
  VW::generate_interactions<R, S, T>(all, ec, dat, num_interacted_features);
}

}  // namespace INTERACTIONS

namespace GD
{

using gd = VW::reductions::gd;

// iterate through one namespace (or its part), callback function FuncT(some_data_R, feature_value_x, feature_weight)
template <class DataT, class WeightOrIndexT, void (*FuncT)(DataT&, float, WeightOrIndexT)>
VW_DEPRECATED("Moved to VW namespace")
inline void foreach_feature(VW::workspace& all, VW::example& ec, DataT& dat)
{
  VW::foreach_feature<DataT, WeightOrIndexT, FuncT>(all, ec, dat);
}

// iterate through one namespace (or its part), callback function FuncT(some_data_R, feature_value_x, feature_weight)
template <class DataT, class WeightOrIndexT, void (*FuncT)(DataT&, float, WeightOrIndexT)>
VW_DEPRECATED("Moved to VW namespace")
inline void foreach_feature(VW::workspace& all, VW::example& ec, DataT& dat, size_t& num_interacted_features)
{
  VW::foreach_feature<DataT, WeightOrIndexT, FuncT>(all, ec, dat, num_interacted_features);
}

// iterate through all namespaces and quadratic&cubic features, callback function T(some_data_R, feature_value_x,
// feature_weight)
template <class DataT, void (*FuncT)(DataT&, float, float&)>
VW_DEPRECATED("Moved to VW namespace")
inline void foreach_feature(VW::workspace& all, VW::example& ec, DataT& dat)
{
  VW::foreach_feature<DataT, float&, FuncT>(all, ec, dat);
}

template <class DataT, void (*FuncT)(DataT&, float, float)>
VW_DEPRECATED("Moved to VW namespace")
inline void foreach_feature(VW::workspace& all, VW::example& ec, DataT& dat)
{
  VW::foreach_feature<DataT, float, FuncT>(all, ec, dat);
}

template <class DataT, void (*FuncT)(DataT&, float, float&)>
VW_DEPRECATED("Moved to VW namespace")
inline void foreach_feature(VW::workspace& all, VW::example& ec, DataT& dat, size_t& num_interacted_features)
{
  VW::foreach_feature<DataT, float&, FuncT>(all, ec, dat, num_interacted_features);
}

template <class DataT, void (*FuncT)(DataT&, float, const float&)>
VW_DEPRECATED("Moved to VW namespace")
inline void foreach_feature(VW::workspace& all, VW::example& ec, DataT& dat, size_t& num_interacted_features)
{
  VW::foreach_feature<DataT, const float&, FuncT>(all, ec, dat, num_interacted_features);
}

VW_DEPRECATED("Moved to VW namespace")
inline float inline_predict(VW::workspace& all, VW::example& ec) { return VW::inline_predict(all, ec); }

VW_DEPRECATED("Moved to VW namespace")
inline float inline_predict(VW::workspace& all, VW::example& ec, size_t& num_generated_features)
{
  return VW::inline_predict(all, ec, num_generated_features);
}

VW_DEPRECATED("Moved to VW namespace")
inline float trunc_weight(const float w, const float gravity) { return VW::trunc_weight(w, gravity); }
}  // namespace GD