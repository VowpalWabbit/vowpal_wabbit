#pragma once

#include "vw.net.native.h"
#include "vw.net.workspace.h"
#include "vw.h"

namespace vw_net_native {

struct builder_context
{
  example* ex;

  features* feature_data;
  unsigned char feature_group;
};

}

extern "C" {
  API int SetupExample(vw_net_native::workspace_context* vw, example* ex, VW::experimental::api_status* status);

  API vw_net_native::builder_context* CreateBuilder(vw_net_native::workspace_context* vw, example* ex, unsigned char feature_group);
  API void DeleteBuilder(vw_net_native::builder_context* builder);

  API void BuilderPreallocate(vw_net_native::builder_context* builder, vw_net_native::dotnet_size_t size);
  API void BuilderAddFeature(vw_net_native::builder_context* builder, uint64_t weight_index, float x);
  API void BuilderAddFeaturesUnchecked(vw_net_native::builder_context* builder, uint64_t weight_index_base, float* begin, float* end);
  API size_t BuilderGetFeatureCount(vw_net_native::builder_context* builder);
}
