#include "vw.net.builders.h"

#include "vw.net.workspace.h"

#include <algorithm>

namespace
{
// Iteration over an example -- VW::empty_example, VW::copy_example_data, VW::setup_example -- is driven
// entirely by ex.indices, so a feature group that holds data while its index is unregistered is invisible
// to all three. Keeping the two in sync is what makes a builder-owned example safe to reset, copy, or set
// up at any point during construction.
void sync_namespace_index(VW::example& ex, unsigned char feature_group)
{
  const auto it = std::find(ex.indices.begin(), ex.indices.end(), feature_group);
  const bool has_features = ex.feature_space[feature_group].size() > 0;

  if (has_features && it == ex.indices.end()) { ex.indices.push_back(feature_group); }
  else if (!has_features && it != ex.indices.end()) { ex.indices.erase(it); }
}
}  // namespace

API int SetupExample(vw_net_native::workspace_context* vw, VW::example* ex, VW::experimental::api_status* status)
{
  try
  {
    VW::setup_example(*vw->vw, ex);
    return VW::experimental::error_code::success;
  }
  CATCH_RETURN_STATUS
}

API int CopyExample(vw_net_native::workspace_context* vw, VW::example* dst, const VW::example* src,
    VW::experimental::api_status* status)
{
  try
  {
    // VW::copy_example_data only overwrites the feature groups named in src->indices, so anything the
    // destination is still carrying has to go first. This also drops any stale namespace index that
    // copy_example_data would otherwise leave orphaned.
    VW::empty_example(*vw->vw, *dst);

    // VW::copy_example_metadata overwrites passthrough unconditionally without releasing what was
    // already there. The destination owns its own passthrough, so release it here.
    delete dst->passthrough;
    dst->passthrough = nullptr;

    VW::copy_example_data_with_label(dst, src);

    // Not covered by copy_example_metadata, but a copy that silently dropped the simple label's
    // `initial` value (or any other reduction's per-example state) would not be a copy.
    dst->ex_reduction_features = src->ex_reduction_features;

    return VW::experimental::error_code::success;
  }
  CATCH_RETURN_STATUS
}

API vw_net_native::builder_context* CreateBuilder(
    vw_net_native::workspace_context* vw, VW::example* ex, unsigned char feature_group)
{
  vw_net_native::builder_context* builder = new vw_net_native::builder_context();
  builder->ex = ex;
  builder->feature_group = feature_group;

  builder->feature_data = ex->feature_space.data() + feature_group;

  // Register the namespace up front rather than at DeleteBuilder time. Otherwise every feature written
  // before the builder is disposed lives in a group the example does not know about: CreateExample ->
  // VW::setup_example would skip it (leaving its weight indices unscaled by the stride multiplier), and
  // an example returned to the pool mid-build would keep the data through VW::empty_example and leak it
  // into the next caller. DeleteBuilder drops the index again if nothing was added.
  if (std::find(ex->indices.begin(), ex->indices.end(), feature_group) == ex->indices.end())
  {
    ex->indices.push_back(feature_group);
  }

  return builder;
}

API void DeleteBuilder(vw_net_native::builder_context* builder)
{
  // It may not be great that the API relies on Disposal to "finish" writing the data to
  // the example. In the future, this should probably be an explicit Build() operation
  // which actually writes data to the example.
  VW::example& ex = *builder->ex;
  const unsigned char feature_group = builder->feature_group;

  // Multiple builders can be open on the same feature group at once, so reconcile against what the
  // group actually holds rather than against what this builder contributed.
  sync_namespace_index(ex, feature_group);

  if (builder->feature_data->size() > 0)
  {
    // Since we switched to using is_newline to track this, we own managing this.
    ex.is_newline = false;
  }

  delete builder;
}

API void BuilderPreallocate(vw_net_native::builder_context* builder, vw_net_native::dotnet_size_t size)
{
  // For sanity, though we will throw on negatives on the other side. Avoid accidentally
  // trying to allocate 2 GB.
  if (size < 0) { return; }

  size_t native_size = static_cast<size_t>(size);
  VW::features& features = *builder->feature_data;

  features.values.reserve(native_size);
  features.indices.reserve(native_size);
}

API void BuilderAddFeature(vw_net_native::builder_context* builder, uint64_t weight_index, float x)
{
  if (x != 0) { builder->feature_data->push_back(x, weight_index); }
}

API void BuilderAddFeatures(vw_net_native::builder_context* builder, const uint64_t* weight_indices,
    const float* values, vw_net_native::dotnet_size_t count)
{
  if (count <= 0) { return; }

  VW::features& features = *builder->feature_data;
  size_t native_count = static_cast<size_t>(count);

  features.values.reserve(features.values.size() + native_count);
  features.indices.reserve(features.indices.size() + native_count);

  for (size_t i = 0; i < native_count; i++)
  {
    // Filter out 0-values, matching BuilderAddFeature.
    if (values[i] != 0) { features.push_back(values[i], weight_indices[i]); }
  }
}

API void BuilderAddFeaturesUnchecked(
    vw_net_native::builder_context* builder, uint64_t weight_index_base, float* begin, float* end)
{
  VW::features& features = *builder->feature_data;

  for (; begin != end; begin++)
  {
    float x = *begin;
    if (x != 0)
    {
      features.values.push_back_unchecked(x);
      features.indices.push_back_unchecked(weight_index_base);
    }

    weight_index_base++;
  }
}

API size_t BuilderGetFeatureCount(vw_net_native::builder_context* builder) { return builder->feature_data->size(); }
