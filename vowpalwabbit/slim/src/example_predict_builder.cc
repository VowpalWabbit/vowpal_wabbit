#include "vw/slim/example_predict_builder.h"

#include "vw/common/hash.h"
#include "vw/core/hashstring.h"

namespace vw_slim
{
example_predict_builder::example_predict_builder(
    VW::example_predict* ex, const char* namespace_name, uint32_t feature_index_num_bits)
    : _ex(ex)
{
  _feature_index_bit_mask = ((uint64_t)1 << feature_index_num_bits) - 1;
  add_namespace(namespace_name[0]);
  _namespace_hash = VW::details::hashstring(namespace_name, strlen(namespace_name), 0);
}

example_predict_builder::example_predict_builder(
    VW::example_predict* ex, VW::namespace_index namespace_idx, uint32_t feature_index_num_bits)
    : _ex(ex), _namespace_hash(namespace_idx)
{
  _feature_index_bit_mask = ((uint64_t)1 << feature_index_num_bits) - 1;
  add_namespace(namespace_idx);
}

void example_predict_builder::add_namespace(VW::namespace_index feature_group)
{
  _namespace_idx = feature_group;
  const auto it = std::find(_ex->indices.begin(), _ex->indices.end(), feature_group);
  if (it == _ex->indices.end()) { _ex->indices.push_back(feature_group); }
}

void example_predict_builder::push_feature_string(const char* feature_name, VW::feature_value value)
{
  VW::feature_index feature_hash =
      _feature_index_bit_mask & VW::details::hashstring(feature_name, strlen(feature_name), _namespace_hash);
  _ex->feature_space[_namespace_idx].push_back(value, feature_hash);
}

void example_predict_builder::push_feature(VW::feature_index feature_idx, VW::feature_value value)
{
  _ex->feature_space[_namespace_idx].push_back(value, _namespace_hash + feature_idx);
}
}  // namespace vw_slim
