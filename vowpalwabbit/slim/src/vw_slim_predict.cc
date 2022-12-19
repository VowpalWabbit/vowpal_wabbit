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
  }
  else { _remove_ns = false; }
}

namespace_copy_guard::~namespace_copy_guard()
{
  _ex.indices.pop_back();
  if (_remove_ns) { _ex.feature_space[_ns].clear(); }
}

void namespace_copy_guard::feature_push_back(VW::feature_value v, VW::feature_index idx)
{
  _ex.feature_space[_ns].push_back(v, idx);
}

feature_offset_guard::feature_offset_guard(VW::example_predict& ex, uint64_t ft_offset)
    : _ex(ex), _old_ft_offset(ex.ft_offset)
{
  _ex.ft_offset = ft_offset;
}

feature_offset_guard::~feature_offset_guard() { _ex.ft_offset = _old_ft_offset; }

stride_shift_guard::stride_shift_guard(VW::example_predict& ex, uint64_t shift) : _ex(ex), _shift(shift)
{
  if (_shift > 0)
  {
    for (auto ns : _ex.indices)
    {
      for (auto& f : _ex.feature_space[ns]) { f.index() <<= _shift; }
    }
  }
}

stride_shift_guard::~stride_shift_guard()
{
  if (_shift > 0)
  {
    for (auto ns : _ex.indices)
    {
      for (auto& f : _ex.feature_space[ns]) { f.index() >>= _shift; }
    }
  }
}

}  // namespace vw_slim