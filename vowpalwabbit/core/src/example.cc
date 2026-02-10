// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "vw/core/example.h"

#include "vw/cache_parser/parse_example_cache.h"
#include "vw/core/cb_continuous_label.h"
#include "vw/core/interactions.h"
#include "vw/core/model_utils.h"
#include "vw/core/reductions/gd.h"
#include "vw/core/simple_label_parser.h"
#include "vw/core/text_utils.h"
#include "vw/core/vw.h"

#include <algorithm>
#include <climits>
#include <cstdint>

void VW::swap_prediction(VW::polyprediction& a, VW::polyprediction& b, VW::prediction_type_t prediction_type)
{
  switch (prediction_type)
  {
    case VW::prediction_type_t::SCALAR:
      std::swap(b.scalar, a.scalar);
      break;
    case VW::prediction_type_t::SCALARS:
      std::swap(b.scalars, a.scalars);
      break;
    case VW::prediction_type_t::ACTION_SCORES:
      std::swap(b.a_s, a.a_s);
      break;
    case VW::prediction_type_t::PDF:
      std::swap(b.pdf, a.pdf);
      break;
    case VW::prediction_type_t::ACTION_PROBS:
      std::swap(b.a_s, a.a_s);
      break;
    case VW::prediction_type_t::MULTICLASS:
      std::swap(b.multiclass, a.multiclass);
      break;
    case VW::prediction_type_t::MULTILABELS:
      std::swap(b.multilabels, a.multilabels);
      break;
    case VW::prediction_type_t::PROB:
      std::swap(b.prob, a.prob);
      break;
    case VW::prediction_type_t::MULTICLASS_PROBS:
      std::swap(b.scalars, a.scalars);
      break;
    case VW::prediction_type_t::DECISION_PROBS:
      std::swap(b.decision_scores, a.decision_scores);
      break;
    case VW::prediction_type_t::ACTION_PDF_VALUE:
      std::swap(b.pdf_value, a.pdf_value);
      break;
    case VW::prediction_type_t::ACTIVE_MULTICLASS:
      std::swap(b.active_multiclass, a.active_multiclass);
      break;
    case VW::prediction_type_t::NOPRED:
      // Noop
      break;
  }
}

float calculate_total_sum_features_squared(bool permutations, VW::example& ec)
{
  float sum_features_squared = 0.f;
  for (const VW::features& fs : ec) { sum_features_squared += fs.sum_feat_sq; }

  float calculated_sum_features_squared = VW::eval_sum_ft_squared_of_generated_ft(
      permutations, *ec.interactions, *ec.extent_interactions, ec.feature_space);
  sum_features_squared += calculated_sum_features_squared;
  return sum_features_squared;
}

VW::example::~example()
{
  if (passthrough)
  {
    delete passthrough;
    passthrough = nullptr;
  }
}

float VW::example::get_total_sum_feat_sq()
{
  if (!_total_sum_feat_sq_calculated)
  {
    total_sum_feat_sq = calculate_total_sum_features_squared(_use_permutations, *this);
    _total_sum_feat_sq_calculated = true;
  }
  return total_sum_feat_sq;
}

float collision_cleanup(VW::features& fs)
{
  // Input must be sorted.
  assert(std::is_sorted(fs.indices.begin(), fs.indices.end()));

  // This loops over the sequence of feature values and their indexes
  // when an index is repeated this combines them by adding their values.
  // This assumes that fs is sorted (which is the case in `flatten_sort_example`).

  float sum_sq = 0.f;
  if (!fs.empty())
  {
    VW::features::iterator p1 = fs.begin();
    uint64_t last_index = p1.index();

    for (VW::features::iterator p2 = (fs.begin() + 1); p2 != fs.end(); ++p2)
    {
      if (last_index == p2.index()) { p1.value() += p2.value(); }
      else
      {
        sum_sq += p1.value() * p1.value();
        ++p1;
        p1.value() = p2.value();
        p1.index() = p2.index();
        last_index = p2.index();
      }
    }

    sum_sq += p1.value() * p1.value();
    ++p1;

    fs.truncate_to(p1, 0);
    fs.sum_feat_sq = sum_sq;
  }

  return sum_sq;
}

namespace VW
{

}  // namespace VW

namespace VW
{

}  // namespace VW

class full_features_and_source
{
public:
  VW::features fs;
  uint32_t stride_shift;
  uint64_t mask;
};

void vec_ffs_store(full_features_and_source& p, float fx, uint64_t fi)
{
  p.fs.push_back(fx, (fi >> p.stride_shift) & p.mask);
}
namespace VW
{

void flatten_features(VW::workspace& all, example& ec, features& fs)
{
  fs.clear();
  full_features_and_source ffs;
  ffs.fs = std::move(fs);
  ffs.stride_shift = all.weights.stride_shift();
  if (all.weights.not_null())
  {
    // all.weights may not be initialized when flatten_features is called during setup.
    ffs.mask = all.weights.mask() >> all.weights.stride_shift();
  }
  else { ffs.mask = all.runtime_state.parse_mask >> all.weights.stride_shift(); }
  VW::foreach_feature<full_features_and_source, uint64_t, vec_ffs_store>(all, ec, ffs);
  ffs.fs.sort(all.runtime_state.parse_mask);
  ffs.fs.sum_feat_sq = collision_cleanup(ffs.fs);
  fs = std::move(ffs.fs);
}

void return_multiple_example(VW::workspace& all, VW::multi_ex& examples)
{
  for (auto ec : examples) { details::clean_example(all, *ec); }
  examples.clear();
}
namespace details
{
void clean_example(VW::workspace& all, example& ec)
{
  VW::empty_example(all, ec);
  all.parser_runtime.example_parser->example_pool.return_object(&ec);
}
void truncate_example_namespace(VW::example& ec, VW::namespace_index ns, const features& fs)
{
  // print_update is called after this del_example_namespace,
  // so we need to keep the ec.num_features correct,
  // so shared features are included in the reported number of "current features"
  // ec.num_features -= numf;
  features& del_target = ec.feature_space[static_cast<size_t>(ns)];
  assert(del_target.size() >= fs.size());
  assert(!ec.indices.empty());
  if (ec.indices.back() == ns && ec.feature_space[static_cast<size_t>(ns)].size() == fs.size())
  {
    ec.indices.pop_back();
  }
  ec.reset_total_sum_feat_sq();
  ec.num_features -= fs.size();
  del_target.truncate_to(del_target.size() - fs.size(), fs.sum_feat_sq);
}

void append_example_namespace(VW::example& ec, VW::namespace_index ns, const features& fs)
{
  const auto index_it = std::find(ec.indices.begin(), ec.indices.end(), ns);
  const bool has_ns = index_it != ec.indices.end();
  if (!has_ns) { ec.indices.push_back(ns); }

  features& add_fs = ec.feature_space[static_cast<size_t>(ns)];
  add_fs.concat(fs);
  ec.reset_total_sum_feat_sq();
  ec.num_features += fs.size();
}

void append_example_namespaces_from_example(VW::example& target, const VW::example& source)
{
  for (VW::namespace_index idx : source.indices)
  {
    if (idx == VW::details::CONSTANT_NAMESPACE) { continue; }
    append_example_namespace(target, idx, source.feature_space[idx]);
  }
}

void truncate_example_namespaces_from_example(VW::example& target, const VW::example& source)
{
  if (source.indices.empty())
  {  // making sure we can deal with empty shared example
    return;
  }
  auto idx = source.indices.end();
  idx--;
  for (; idx >= source.indices.begin(); idx--)
  {
    if (*idx == VW::details::CONSTANT_NAMESPACE) { continue; }
    truncate_example_namespace(target, *idx, source.feature_space[*idx]);
  }
}
}  // namespace details
}  // namespace VW

namespace VW
{
std::string to_string(const v_array<float>& scalars, int decimal_precision)
{
  std::stringstream ss;
  std::string delim;
  for (float f : scalars)
  {
    ss << delim << VW::fmt_float(f, decimal_precision);
    delim = ",";
  }
  return ss.str();
}
}  // namespace VW
