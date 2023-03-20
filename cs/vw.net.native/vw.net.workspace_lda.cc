#include "vw.net.workspace_lda.h"

#include "vw/config/options.h"
#include "vw/core/reductions/lda_core.h"

API int WorkspaceGetTopicCount(vw_net_native::workspace_context* workspace)
{
  return (int)workspace->vw->reduction_state.lda;
}

API uint64_t WorkspaceGetTopicSize(vw_net_native::workspace_context* workspace)
{
  return 1ULL << workspace->vw->initial_weights_config.num_bits;
}

template <typename T>
int64_t fill_topic_allocation(vw_net_native::workspace_context* workspace, T& weights, float** topic_weight_buffers,
    size_t buffer_size, size_t buffers_count)
{
  int topic_count = (int)workspace->vw->reduction_state.lda;
  uint64_t topic_size = WorkspaceGetTopicSize(workspace);
  vw_net_native::dotnet_size_t returned = static_cast<vw_net_native::dotnet_size_t>(topic_count * topic_size);

  if (topic_count > buffers_count || topic_size > buffer_size) { return -returned; }

  // TODO: better way of peaking into lda?
  auto lda_rho = workspace->vw->options->get_typed_option<float>("lda_rho").value();

  for (auto iter = weights.begin(); iter != weights.end(); ++iter)
  {
    VW::weight* wp = &(*iter);
    for (uint64_t k = 0; k < topic_count; k++) { topic_weight_buffers[(int)k][(int)iter.index()] = wp[k] + lda_rho; }
  }

  return returned;
}

API int64_t WorkspaceFillTopicAllocation(vw_net_native::workspace_context* workspace, float** topic_weight_buffers,
    vw_net_native::dotnet_size_t buffer_size, vw_net_native::dotnet_size_t buffer_count)
{
  if (workspace->vw->weights.sparse)
  {
    return fill_topic_allocation(
        workspace, workspace->vw->weights.sparse_weights, topic_weight_buffers, buffer_size, buffer_count);
  }
  else
  {
    return fill_topic_allocation(
        workspace, workspace->vw->weights.dense_weights, topic_weight_buffers, buffer_size, buffer_count);
  }
}

API vw_net_native::dotnet_size_t WorkspaceFillSingleTopicTopWeights(vw_net_native::workspace_context* workspace,
    int topic, VW::feature* topic_weight_buffer, vw_net_native::dotnet_size_t buffer_size)
{
  std::vector<VW::feature> top_weights;
  VW::reductions::lda::get_top_weights(workspace->vw, buffer_size, topic, top_weights);

  return vw_net_native::stdvector_copy_to_managed(top_weights, topic_weight_buffer, buffer_size);
}
