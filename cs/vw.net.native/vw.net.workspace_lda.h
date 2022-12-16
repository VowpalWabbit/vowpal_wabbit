#pragma once

#include "vw.net.native.h"
#include "vw.net.workspace.h"
#include "vw/core/feature_group.h"

extern "C"
{
  API int WorkspaceGetTopicCount(vw_net_native::workspace_context* workspace);
  API uint64_t WorkspaceGetTopicSize(vw_net_native::workspace_context* workspace);

  // TODO: This is suboptimal because it requires traversing the data multiple times.
  API int64_t WorkspaceFillTopicAllocation(vw_net_native::workspace_context* workspace, float** topic_weight_buffers,
      vw_net_native::dotnet_size_t buffer_size, vw_net_native::dotnet_size_t buffer_count);
  API vw_net_native::dotnet_size_t WorkspaceFillSingleTopicTopWeights(vw_net_native::workspace_context* workspace,
      VW::feature* topic_weight_buffer, vw_net_native::dotnet_size_t buffer_size);
}
