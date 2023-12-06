#include "vw/slim/vw_slim_predict.h"

#include <algorithm>
#include <cctype>

namespace vw_slim
{
uint64_t ceil_log_2(uint64_t v)
{
  if (v == 0) { return 0; }
  else { return 1 + ceil_log_2(v >> 1); }
}

namespace_copy_guard::namespace_copy_guard(VW::example_predict& ex, unsigned char ns) : _ex(ex), _ns(ns)
{
  if (std::end(_ex.indices) == std::find(std::begin(_ex.indices), std::end(_ex.indices), ns))
  {
    _ex.indices.push_back(_ns);
    _remove_ns = true;
    _old_size = 0;
  }
  else
  {
    _remove_ns = false;
    _old_size = _ex.feature_space[_ns].size();
  }
}

namespace_copy_guard::~namespace_copy_guard()
{
  if (_remove_ns)
  {
    _ex.feature_space[_ns].clear();
    _ex.indices.pop_back();
  }
  else { _ex.feature_space[_ns].truncate_to(_old_size); }
}

void namespace_copy_guard::feature_push_back(VW::feature_value v, VW::feature_index idx)
{
  _ex.feature_space[_ns].push_back(v, idx);
}

feature_offset_guard::feature_offset_guard(VW::example_predict& ex, uint64_t ft_index_offset)
    : _ex(ex), _old_ft_index_offset(ex.ft_offset)
{
  _ex.ft_offset = ft_index_offset;
}

feature_offset_guard::~feature_offset_guard() { _ex.ft_offset = _old_ft_index_offset; }

feature_scale_guard::feature_scale_guard(VW::example_predict& ex, uint64_t ft_index_scale)
    : _ex(ex), _ft_index_scale(ft_index_scale)
{
  if (_ft_index_scale > 1)
  {
    for (auto ns : _ex.indices)
    {
      for (auto& f : _ex.feature_space[ns]) { f.index() *= _ft_index_scale; }
    }
  }
}

feature_scale_guard::~feature_scale_guard()
{
  if (_ft_index_scale > 1)
  {
    for (auto ns : _ex.indices)
    {
      for (auto& f : _ex.feature_space[ns]) { f.index() /= _ft_index_scale; }
    }
  }
}

}  // namespace vw_slim