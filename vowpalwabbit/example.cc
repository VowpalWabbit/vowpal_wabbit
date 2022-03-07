// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include <cstdint>
#include <algorithm>

#include "cb_continuous_label.h"
#include "example.h"
#include "reductions/gd.h"
#include "simple_label_parser.h"
#include "reductions/interactions.h"
#include "model_utils.h"
#include "text_utils.h"

float calculate_total_sum_features_squared(bool permutations, example& ec)
{
  float sum_features_squared = 0.f;
  for (const features& fs : ec) { sum_features_squared += fs.sum_feat_sq; }

  float calculated_sum_features_squared = INTERACTIONS::eval_sum_ft_squared_of_generated_ft(
      permutations, *ec.interactions, *ec.extent_interactions, ec.feature_space);
  sum_features_squared += calculated_sum_features_squared;
  return sum_features_squared;
}

example::~example()
{
  if (passthrough)
  {
    delete passthrough;
    passthrough = nullptr;
  }
}

float collision_cleanup(features& fs)
{
  uint64_t last_index = static_cast<uint64_t>(-1);
  float sum_sq = 0.f;
  features::iterator pos = fs.begin();
  for (features::iterator& f : fs)
  {
    if (last_index == f.index())
      pos.value() += f.value();
    else
    {
      sum_sq += pos.value() * pos.value();
      ++pos;
      pos.value() = f.value();
      pos.index() = f.index();
      last_index = f.index();
    }
  }

  sum_sq += pos.value() * pos.value();
  ++pos;
  // Don't change the sum_feat_sq as we will do it manually directly after.
  fs.truncate_to(pos, 0);
  fs.sum_feat_sq = sum_sq;

  return sum_sq;
}

namespace VW
{
void copy_example_label(example* dst, example* src, void (*)(polylabel*, polylabel*))
{
  dst->l = src->l;
  dst->_reduction_features = src->_reduction_features;
}

void copy_example_label(example* dst, const example* src) { dst->l = src->l; }

void copy_example_metadata(example* dst, const example* src)
{
  dst->tag = src->tag;
  dst->example_counter = src->example_counter;

  dst->ft_offset = src->ft_offset;

  dst->partial_prediction = src->partial_prediction;
  if (src->passthrough == nullptr)
    dst->passthrough = nullptr;
  else
  {
    dst->passthrough = new features(*src->passthrough);
  }
  dst->loss = src->loss;
  dst->weight = src->weight;
  dst->confidence = src->confidence;
  dst->test_only = src->test_only;
  dst->end_pass = src->end_pass;
  dst->is_newline = src->is_newline;
  dst->sorted = src->sorted;
#ifdef PRIVACY_ACTIVATION
  dst->tag_hash = src->tag_hash;
#endif
}

void copy_example_data(example* dst, const example* src)
{
  copy_example_metadata(dst, src);

  // copy feature data
  dst->indices = src->indices;
  for (namespace_index c : src->indices) dst->feature_space[c] = src->feature_space[c];
  dst->num_features = src->num_features;
  dst->total_sum_feat_sq = src->total_sum_feat_sq;
  dst->total_sum_feat_sq_calculated = src->total_sum_feat_sq_calculated;
  dst->use_permutations = src->use_permutations;
  dst->interactions = src->interactions;
  dst->extent_interactions = src->extent_interactions;
  dst->_debug_current_reduction_depth = src->_debug_current_reduction_depth;
}

void copy_example_data_with_label(example* dst, const example* src)
{
  copy_example_data(dst, src);
  copy_example_label(dst, src);
}

void move_feature_namespace(example* dst, example* src, namespace_index c)
{
  if (std::find(src->indices.begin(), src->indices.end(), c) == src->indices.end()) return;  // index not present in src
  if (std::find(dst->indices.begin(), dst->indices.end(), c) == dst->indices.end()) dst->indices.push_back(c);

  auto& fdst = dst->feature_space[c];
  auto& fsrc = src->feature_space[c];

  src->num_features -= fsrc.size();
  src->reset_total_sum_feat_sq();
  std::swap(fdst, fsrc);
  dst->num_features += fdst.size();
  dst->reset_total_sum_feat_sq();
}

}  // namespace VW

struct features_and_source
{
  v_array<feature> feature_map;  // map to store sparse feature vectors
  uint32_t stride_shift;
  uint64_t mask;
};

void vec_store(features_and_source& p, float fx, uint64_t fi)
{
  p.feature_map.push_back(feature(fx, (fi >> p.stride_shift) & p.mask));
}

namespace VW
{
feature* get_features(VW::workspace& all, example* ec, size_t& feature_map_len)
{
  features_and_source fs;
  fs.stride_shift = all.weights.stride_shift();
  fs.mask = all.weights.mask() >> all.weights.stride_shift();
  GD::foreach_feature<features_and_source, uint64_t, vec_store>(all, *ec, fs);

  feature_map_len = fs.feature_map.size();
  return fs.feature_map.begin();
}

void return_features(feature* f) { free_it(f); }
}  // namespace VW

struct full_features_and_source
{
  features fs;
  uint32_t stride_shift;
  uint64_t mask;
};

void vec_ffs_store(full_features_and_source& p, float fx, uint64_t fi)
{
  p.fs.push_back(fx, (fi >> p.stride_shift) & p.mask);
}

flat_example* flatten_example(VW::workspace& all, example* ec)
{
  flat_example& fec = calloc_or_throw<flat_example>();
  fec.l = ec->l;
  fec._reduction_features = ec->_reduction_features;

  fec.tag_len = ec->tag.size();
  if (fec.tag_len > 0)
  {
    fec.tag = calloc_or_throw<char>(fec.tag_len + 1);
    memcpy(fec.tag, ec->tag.begin(), fec.tag_len);
  }

  fec.example_counter = ec->example_counter;
  fec.ft_offset = ec->ft_offset;
  fec.num_features = ec->num_features;

  full_features_and_source ffs;
  ffs.stride_shift = all.weights.stride_shift();
  if (all.weights.not_null())  // TODO:temporary fix. all.weights is not initialized at this point in some cases.
    ffs.mask = all.weights.mask() >> all.weights.stride_shift();
  else
    ffs.mask = static_cast<uint64_t>(LONG_MAX) >> all.weights.stride_shift();
  GD::foreach_feature<full_features_and_source, uint64_t, vec_ffs_store>(all, *ec, ffs);

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
    if (fec->tag_len > 0) free(fec->tag);
    free(fec);
  }
}

namespace VW
{
example* alloc_examples(size_t count)
{
  example* ec = calloc_or_throw<example>(count);
  if (ec == nullptr) { return nullptr; }
  for (size_t i = 0; i < count; i++) { new (ec + i) example; }
  return ec;
}

void dealloc_examples(example* example_ptr, size_t count)
{
  for (size_t i = 0; i < count; i++) { (example_ptr + i)->~example(); }
  free(example_ptr);
}

void finish_example(VW::workspace&, example&);
void clean_example(VW::workspace&, example&);

void finish_example(VW::workspace& all, multi_ex& ec_seq)
{
  for (example* ecc : ec_seq) VW::finish_example(all, *ecc);
}

void return_multiple_example(VW::workspace& all, v_array<example*>& examples)
{
  for (auto ec : examples) { clean_example(all, *ec); }
  examples.clear();
}

namespace model_utils
{
size_t read_model_field(io_buf& io, flat_example& fe, label_parser& lbl_parser)
{
  size_t bytes = 0;
  bool tag_is_null;
  bytes += lbl_parser.read_cached_label(fe.l, fe._reduction_features, io);
  bytes += read_model_field(io, fe.tag_len);
  bytes += read_model_field(io, tag_is_null);
  if (!tag_is_null) { bytes += read_model_field(io, *fe.tag); }
  bytes += read_model_field(io, fe.example_counter);
  bytes += read_model_field(io, fe.ft_offset);
  bytes += read_model_field(io, fe.global_weight);
  bytes += read_model_field(io, fe.num_features);
  bytes += read_model_field(io, fe.total_sum_feat_sq);
  unsigned char index = 0;
  char* c;
  bytes += read_cached_index(io, index, c);
  bool sorted = true;
  bytes += read_cached_features(io, fe.fs, sorted, c);
  return bytes;
}
size_t write_model_field(io_buf& io, const flat_example& fe, const std::string& upstream_name, bool text,
    label_parser& lbl_parser, uint64_t parse_mask)
{
  size_t bytes = 0;
  lbl_parser.cache_label(fe.l, fe._reduction_features, io, upstream_name + "_label", text);
  bytes += write_model_field(io, fe.tag_len, upstream_name + "_tag_len", text);
  bytes += write_model_field(io, fe.tag == nullptr, upstream_name + "_tag_is_null", text);
  if (!(fe.tag == nullptr)) { bytes += write_model_field(io, *fe.tag, upstream_name + "_tag", text); }
  bytes += write_model_field(io, fe.example_counter, upstream_name + "_example_counter", text);
  bytes += write_model_field(io, fe.ft_offset, upstream_name + "_ft_offset", text);
  bytes += write_model_field(io, fe.global_weight, upstream_name + "_global_weight", text);
  bytes += write_model_field(io, fe.num_features, upstream_name + "_num_features", text);
  bytes += write_model_field(io, fe.total_sum_feat_sq, upstream_name + "_total_sum_feat_sq", text);
  char* c;
  cache_index(io, 0, fe.fs, c);
  cache_features(io, fe.fs, parse_mask, c);
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