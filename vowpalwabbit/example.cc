// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include <cstdint>
#include <algorithm>

#include "example.h"
#include "gd.h"

VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_DEPRECATED_USAGE
example::example()
{
  memset(&l, 0, sizeof(polylabel));
  memset(&pred, 0, sizeof(polyprediction));
  tag = v_init<char>();
}
VW_WARNING_STATE_POP

VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_DEPRECATED_USAGE
example::~example()
{
  tag.delete_v();
  if (passthrough)
  {
    delete passthrough;
    passthrough = nullptr;
  }
}
VW_WARNING_STATE_POP

VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_DEPRECATED_USAGE
example::example(example&& other) noexcept
    : example_predict(std::move(other))
    , l(other.l)
    , pred(other.pred)
    , weight(other.weight)
    , tag(std::move(other.tag))
    , example_counter(other.example_counter)
    , num_features(other.num_features)
    , partial_prediction(other.partial_prediction)
    , updated_prediction(other.updated_prediction)
    , loss(other.loss)
    , total_sum_feat_sq(other.total_sum_feat_sq)
    , confidence(other.confidence)
    , passthrough(other.passthrough)
    , test_only(other.test_only)
    , end_pass(other.end_pass)
    , sorted(other.sorted)
    , in_use(other.in_use)
{
  other.weight = 1.f;
  auto& other_tag = other.tag;
  other_tag._begin = nullptr;
  other_tag._end = nullptr;
  other_tag.end_array = nullptr;
  other.example_counter = 0;
  other.num_features = 0;
  other.partial_prediction = 0.f;
  other.updated_prediction = 0.f;
  other.loss = 0.f;
  other.total_sum_feat_sq = 0.f;
  other.confidence = 0.f;
  other.passthrough = nullptr;
  other.test_only = false;
  other.end_pass = false;
  other.sorted = false;
  other.in_use = false;
}
VW_WARNING_STATE_POP

example& example::operator=(example&& other) noexcept
{
  example_predict::operator=(std::move(other));
  l = other.l;
  pred = other.pred;
  weight = other.weight;
  tag = std::move(other.tag);
  example_counter = other.example_counter;
  num_features = other.num_features;
  partial_prediction = other.partial_prediction;
  updated_prediction = other.updated_prediction;
  loss = other.loss;
  total_sum_feat_sq = other.total_sum_feat_sq;
  confidence = other.confidence;
  passthrough = other.passthrough;
  test_only = other.test_only;
  end_pass = other.end_pass;
  sorted = other.sorted;
VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_DEPRECATED_USAGE
  in_use = other.in_use;
VW_WARNING_STATE_POP

  other.weight = 1.f;

  // We need to null out all the v_arrays to prevent double freeing during moves
  auto& other_tag = other.tag;
  other_tag._begin = nullptr;
  other_tag._end = nullptr;
  other_tag.end_array = nullptr;

  other.example_counter = 0;
  other.num_features = 0;
  other.partial_prediction = 0.f;
  other.updated_prediction = 0.f;
  other.loss = 0.f;
  other.total_sum_feat_sq = 0.f;
  other.confidence = 0.f;
  other.passthrough = nullptr;
  other.test_only = false;
  other.end_pass = false;
  other.sorted = false;
VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_DEPRECATED_USAGE
  other.in_use = false;
VW_WARNING_STATE_POP
  return *this;
}

void example::delete_unions(void (*delete_label)(void*), void (*delete_prediction)(void*))
{
  if (delete_label)
  {
    delete_label(&l);
  }

  if (delete_prediction)
  {
    delete_prediction(&pred);
  }
}

float collision_cleanup(features& fs)
{
  uint64_t last_index = (uint64_t)-1;
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
  fs.sum_feat_sq = sum_sq;
  ++pos;
  fs.truncate_to(pos);

  return sum_sq;
}

namespace VW
{
void copy_example_label(example* dst, example* src, size_t, void (*copy_label)(void*, void*))
{
  if (copy_label)
    copy_label(&dst->l, &src->l);  // TODO: we really need to delete_label on dst :(
  else
    dst->l = src->l;
}

void copy_example_metadata(bool /* audit */, example* dst, example* src)
{
  copy_array(dst->tag, src->tag);
  dst->example_counter = src->example_counter;

  dst->ft_offset = src->ft_offset;

  dst->partial_prediction = src->partial_prediction;
  if (src->passthrough == nullptr)
    dst->passthrough = nullptr;
  else
  {
    dst->passthrough = new features;
    dst->passthrough->deep_copy_from(*src->passthrough);
  }
  dst->loss = src->loss;
  dst->weight = src->weight;
  dst->confidence = src->confidence;
  dst->test_only = src->test_only;
  dst->end_pass = src->end_pass;
  dst->sorted = src->sorted;
}

void copy_example_data(bool audit, example* dst, example* src)
{
  // std::cerr << "copy_example_data dst = " << dst << std::endl;
  copy_example_metadata(audit, dst, src);

  // copy feature data
  copy_array(dst->indices, src->indices);
  for (namespace_index c : src->indices) dst->feature_space[c].deep_copy_from(src->feature_space[c]);
  // copy_array(dst->atomics[i], src->atomics[i]);
  dst->num_features = src->num_features;
  dst->total_sum_feat_sq = src->total_sum_feat_sq;
  dst->interactions = src->interactions;
}

void copy_example_data(bool audit, example* dst, example* src, size_t label_size, void (*copy_label)(void*, void*))
{
  copy_example_data(audit, dst, src);
  copy_example_label(dst, src, label_size, copy_label);
}

void move_feature_namespace(example* dst, example* src, namespace_index c)
{
  if (std::find(src->indices.begin(), src->indices.end(), c) == src->indices.end())
    return;  // index not present in src
  if (std::find(dst->indices.begin(), dst->indices.end(), c) == dst->indices.end())
    dst->indices.push_back(c);

  auto& fdst = dst->feature_space[c];
  auto& fsrc = src->feature_space[c];

  src->num_features -= fsrc.size();
  src->total_sum_feat_sq -= fsrc.sum_feat_sq;
  std::swap(fdst, fsrc);
  dst->num_features += fdst.size();
  dst->total_sum_feat_sq += fdst.sum_feat_sq;
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
  p.feature_map.push_back(feature(fx, (uint64_t)(fi >> p.stride_shift) & p.mask));
}

namespace VW
{
feature* get_features(vw& all, example* ec, size_t& feature_map_len)
{
  features_and_source fs;
  fs.stride_shift = all.weights.stride_shift();
  fs.mask = (uint64_t)all.weights.mask() >> all.weights.stride_shift();
  fs.feature_map = v_init<feature>();
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
  p.fs.push_back(fx, (uint64_t)(fi >> p.stride_shift) & p.mask);
}

flat_example* flatten_example(vw& all, example* ec)
{
  flat_example& fec = calloc_or_throw<flat_example>();
  fec.l = ec->l;
  fec.l.simple.weight = ec->weight;

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
    ffs.mask = (uint64_t)all.weights.mask() >> all.weights.stride_shift();
  else
    ffs.mask = (uint64_t)LONG_MAX >> all.weights.stride_shift();
  GD::foreach_feature<full_features_and_source, uint64_t, vec_ffs_store>(all, *ec, ffs);

  std::swap(fec.fs, ffs.fs);

  return &fec;
}

flat_example* flatten_sort_example(vw& all, example* ec)
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
    if (fec->tag_len > 0)
      free(fec->tag);
    free(fec);
  }
}

std::string features_to_string(const example& ec)
{
  std::stringstream strstream;
  strstream << "[off=" << ec.ft_offset << "]";
  for (auto& f : ec.feature_space)
  {
    auto ind_iter = f.indicies.cbegin();
    auto val_iter = f.values.cbegin();
    for (; ind_iter != f.indicies.cend(); ++ind_iter, ++val_iter)
    {
      strstream << "[h=" << *ind_iter << ","
                << "v=" << *val_iter << "]";
    }
  }
  return strstream.str();
}

std::string cb_label_to_string(const example& ec) {
  std::stringstream strstream;
  strstream << "[l.cb={";
  auto& costs = ec.l.cb.costs;
  for (auto c = costs.cbegin(); c != costs.cend(); ++c)
  {
    strstream << "{c=" << c->cost << ",a=" << c->action << ",p=" << c->probability << ",pp=" << c->partial_prediction << "}";
  }
  strstream << "}]";
  return strstream.str();
}

std::string simple_label_to_string(const example& ec)
{
  std::stringstream strstream;
  strstream << "[l=" << ec.l.simple.label << ",w=" << ec.l.simple.weight << "]";
  return strstream.str();
}

std::string depth_indent_string(const example& ec)
{
  return depth_indent_string(ec._current_reduction_depth);
}

std::string depth_indent_string(int32_t stack_depth)
{
  std::stringstream strstream;
  for (auto i = 0; i < stack_depth - 1; i++)
  {
    strstream << "| ";
  }
  strstream << "+ ";
  return strstream.str();
}

std::string scalar_pred_to_string(const example& ec)
{
  std::stringstream strstream;
  strstream << "[p=" << ec.pred.scalar << ", pp=" << ec.partial_prediction << "]";
  return strstream.str();
}

std::string a_s_pred_to_string(const example& ec)
{
  std::stringstream strstream;
  strstream << "ec.pred.a_s[";
  for (uint32_t i = 0; i < ec.pred.a_s.size(); i++)
  {
    strstream << "(" << i << " = " << ec.pred.a_s[i].action << ", " << ec.pred.a_s[i].score << ")";
  }
  strstream << "]";
  return strstream.str();
}

std::string multiclass_pred_to_string(const example& ec)
{
  std::stringstream strstream;
  strstream << "ec.pred.multiclass = " << ec.pred.multiclass;
  return strstream.str();
}

std::string prob_dist_pred_to_string(const example& ec)
{
  std::stringstream strstream;
  strstream << "ec.pred.prob_dist[";
  for (uint32_t i = 0; i < ec.pred.pdf.size(); i++)
  {
    strstream << "(" << i << " = " << ec.pred.pdf[i].left << "-" << ec.pred.pdf[i].right << ", "
              << ec.pred.pdf[i].pdf_value << ")";
  }
  strstream << "]";
  return strstream.str();
}

namespace VW
{
example* alloc_examples(size_t, size_t count = 1)
{
  example* ec = calloc_or_throw<example>(count);
  if (ec == nullptr)
    return nullptr;
  for (size_t i = 0; i < count; i++)
  {
    ec[i].ft_offset = 0;
    //  std::cerr << "  alloc_example.indices.begin()=" << ec->indices.begin() << " end=" << ec->indices.end() << " //
    //  ld = " << ec->ld << "\t|| me = " << ec << std::endl;
  }
  return ec;
}

void dealloc_example(void (*delete_label)(void*), example& ec, void (*delete_prediction)(void*))
{
  ec.delete_unions(delete_label, delete_prediction);
  ec.~example();
}

void finish_example(vw&, example&);
void clean_example(vw&, example&, bool rewind);

void finish_example(vw& all, multi_ex& ec_seq)
{
  for (example* ecc : ec_seq) VW::finish_example(all, *ecc);
}

void return_multiple_example(vw& all, v_array<example*>& examples)
{
  for (auto ec : examples)
  {
    clean_example(all, *ec, true);
  }
  examples.clear();
}

restore_prediction::restore_prediction(example& ec)
: _prediction(ec.pred), _ec(ec) {}

restore_prediction::~restore_prediction()
{ _ec.pred = _prediction; }

}  // namespace VW
