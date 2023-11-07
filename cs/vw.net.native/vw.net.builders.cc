#include "vw.net.builders.h"

#include "vw.net.workspace.h"

API int SetupExample(vw_net_native::workspace_context* vw, VW::example* ex, VW::experimental::api_status* status)
{
  try
  {
    VW::setup_example(*vw->vw, ex);
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

  return builder;
}

API void DeleteBuilder(vw_net_native::builder_context* builder)
{
  // It may not be great that the API relies on Disposal to "finish" writing the data to
  // the example. In the future, this should probably be an explicit Build() operation
  // which actually writes data to the example. Otherwise, while Builders are alive, the
  // example is not necessarily in a "valid" state.
  if (builder->feature_data->size() > 0)
  {
    VW::example& ex = *builder->ex;
    const unsigned char feature_group = builder->feature_group;

    // avoid duplicate insertion
    // can't check at the beginning, because multiple builders can be open
    // at the same time
    auto it = std::find(ex.indices.begin(), ex.indices.end(), feature_group);

    if (it == ex.indices.end())
    {
      ex.indices.push_back(feature_group);

      // Since we switched to using is_newline to track this, we own managing this.
      ex.is_newline = false;
    }
  }
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
