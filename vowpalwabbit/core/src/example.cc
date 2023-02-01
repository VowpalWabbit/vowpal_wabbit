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
flat_example* flatten_example(VW::workspace& all, example* ec)
{
  flat_example& fec = VW::details::calloc_or_throw<flat_example>();
  fec.l = ec->l;
  fec.tag = ec->tag;
  fec.ex_reduction_features = ec->ex_reduction_features;
  fec.example_counter = ec->example_counter;
  fec.ft_offset = ec->ft_offset;
  fec.num_features = ec->num_features;

  full_features_and_source ffs;
  ffs.stride_shift = all.weights.stride_shift();
  if (all.weights.not_null())
  {  // TODO:temporary fix. all.weights is not initialized at this point in some cases.
    ffs.mask = all.weights.mask() >> all.weights.stride_shift();
  }
  else { ffs.mask = static_cast<uint64_t>(LONG_MAX) >> all.weights.stride_shift(); }
  VW::foreach_feature<full_features_and_source, uint64_t, vec_ffs_store>(all, *ec, ffs);

  std::swap(fec.fs, ffs.fs);

  return &fec;
}

flat_example* flatten_sort_example(VW::workspace& all, example* ec)
{
  flat_example* fec = flatten_example(all, ec);
  fec->fs.sort(all.parse_mask);
  fec->total_sum_feat_sq = collision_cleanup(fec->fs);
  return fec;
}

void free_flatten_example(flat_example* fec)
{
  // note: The label memory should be freed by by freeing the original example.
  if (fec)
  {
    fec->fs.~features();
    free(fec);
  }
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
  all.example_parser->example_pool.return_object(&ec);
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

namespace model_utils
{
size_t read_model_field(io_buf& io, flat_example& fe, VW::label_parser& lbl_parser)
{
  size_t bytes = 0;
  lbl_parser.default_label(fe.l);
  bytes += lbl_parser.read_cached_label(fe.l, fe.ex_reduction_features, io);
  bytes += read_model_field(io, fe.tag);
  bytes += read_model_field(io, fe.example_counter);
  bytes += read_model_field(io, fe.ft_offset);
  bytes += read_model_field(io, fe.global_weight);
  bytes += read_model_field(io, fe.num_features);
  bytes += read_model_field(io, fe.total_sum_feat_sq);
  unsigned char index = 0;
  bytes += ::VW::parsers::cache::details::read_cached_index(io, index);
  bool sorted = true;
  bytes += ::VW::parsers::cache::details::read_cached_features(io, fe.fs, sorted);
  return bytes;
}
size_t write_model_field(io_buf& io, const flat_example& fe, const std::string& upstream_name, bool text,
    VW::label_parser& lbl_parser, uint64_t parse_mask)
{
  size_t bytes = 0;
  lbl_parser.cache_label(fe.l, fe.ex_reduction_features, io, upstream_name + "_label", text);
  bytes += write_model_field(io, fe.tag, upstream_name + "_tag", text);
  bytes += write_model_field(io, fe.example_counter, upstream_name + "_example_counter", text);
  bytes += write_model_field(io, fe.ft_offset, upstream_name + "_ft_offset", text);
  bytes += write_model_field(io, fe.global_weight, upstream_name + "_global_weight", text);
  bytes += write_model_field(io, fe.num_features, upstream_name + "_num_features", text);
  bytes += write_model_field(io, fe.total_sum_feat_sq, upstream_name + "_total_sum_feat_sq", text);
  ::VW::parsers::cache::details::cache_index(io, 0);
  ::VW::parsers::cache::details::cache_features(io, fe.fs, parse_mask);
  return bytes;
}
}  // namespace model_utils
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
