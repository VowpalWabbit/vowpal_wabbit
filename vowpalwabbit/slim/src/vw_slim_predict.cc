#include "vw_slim_predict.h"

#include <cctype>
#include <algorithm>

namespace vw_slim
{
uint64_t ceil_log_2(uint64_t v)
{
  if (v == 0)
    return 0;
  else
    return 1 + ceil_log_2(v >> 1);
}

namespace_copy_guard::namespace_copy_guard(example_predict& ex, unsigned char ns_index, uint64_t ns_hash)
    : _ex(ex), _ns_index(ns_index), _ns_hash(ns_hash)
{
  _features = &_ex.feature_space.get_or_create(_ns_index, _ns_hash);
}

namespace_copy_guard::~namespace_copy_guard()
{
  _ex.feature_space.remove(_ns_index, _ns_hash);
  _features = nullptr;
}

void namespace_copy_guard::feature_push_back(feature_value v, feature_index idx)
{ _features->push_back(v, idx); }

feature_offset_guard::feature_offset_guard(example_predict& ex, uint64_t ft_offset)
    : _ex(ex), _old_ft_offset(ex.ft_offset)
{
  _ex.ft_offset = ft_offset;
}

feature_offset_guard::~feature_offset_guard() { _ex.ft_offset = _old_ft_offset; }

stride_shift_guard::stride_shift_guard(example_predict& ex, uint64_t shift) : _ex(ex), _shift(shift)
{
  if (_shift > 0)
  {
    for (auto& group_list : _ex.feature_space)
    {
      for (auto& ns_fs : group_list)
      {
        for (auto& f : ns_fs.feats) { f.index() <<= _shift; }
      }
    }
  }
}

stride_shift_guard::~stride_shift_guard()
{
  if (_shift > 0)
  {
    for (auto& group_list : _ex.feature_space)
    {
      for (auto& ns_fs : group_list)
      {
        for (auto& f : ns_fs.feats) { f.index() >>= _shift; }
      }
    }
  }
}

};  // namespace vw_slim
