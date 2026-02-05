#pragma once

#include "vw.net.native.h"
#include "vw.net.stream_adapter.h"
#include "vw/core/example.h"
#include "vw/core/label_type.h"
#include "vw/core/prediction_type.h"
#include "vw/core/vw.h"
#include "vw/json_parser/decision_service_utils.h"

#include <string>

namespace vw_net_native
{
struct workspace_context
{
  VW::workspace* vw;

  trace_message_t trace_listener;
  void* trace_listener_context;
};

struct performance_statistics_t
{
  size_t total_features;
  size_t examples_per_pass;
  double weighted_examples;
  double weighted_labels;
  double average_loss;
  float best_constant;
  float best_constant_loss;
};

using example_pool_get_example_fn = example& (*)(void*);
using create_prediction_callback_fn = void (*)();

}  // namespace vw_net_native

extern "C"
{
  API vw_net_native::workspace_context* CreateWorkspaceWithSeedVwModel(vw_net_native::workspace_context* seed,
      char* arguments, size_t arguments_size, trace_message_t trace_listener, void* trace_context,
      VW::experimental::api_status* status);
  API vw_net_native::workspace_context* CreateWorkspaceWithModelData(char* arguments, size_t arguments_size,
      vw_net_native::io_reader_vtable model_reader, trace_message_t trace_listener, void* trace_context,
      VW::experimental::api_status* status);
  API vw_net_native::workspace_context* CreateWorkspace(char* arguments, size_t arguments_size,
      trace_message_t trace_listener, void* trace_context, VW::experimental::api_status* status);
  API vw_net_native::ERROR_CODE DeleteWorkspace(
      vw_net_native::workspace_context* workspace, VW::experimental::api_status* status);

  API VW::prediction_type_t WorkspaceGetOutputPredictionType(vw_net_native::workspace_context* workspace);
  API vw_net_native::ERROR_CODE WorkspaceReload(vw_net_native::workspace_context* workspace, char* arguments,
      size_t arguments_size, VW::experimental::api_status* status);
  API vw_net_native::ERROR_CODE WorkspaceSavePredictorToFile(vw_net_native::workspace_context* workspace,
      char* filename, size_t filename_size, VW::experimental::api_status* status);
  API vw_net_native::ERROR_CODE WorkspaceSavePredictorToWriter(vw_net_native::workspace_context* workspace,
      vw_net_native::io_writer_vtable writer, VW::experimental::api_status* status);

  API void WorkspaceGetPerformanceStatistics(
      vw_net_native::workspace_context* workspace, vw_net_native::performance_statistics_t* statistics);

  API size_t WorkspaceHashSpace(vw_net_native::workspace_context* workspace, char* space, size_t space_size);
  API size_t WorkspaceHashFeature(
      vw_net_native::workspace_context* workspace, char* feature, size_t feature_size, size_t space_hash);

  API void WorkspaceSetUpAllReduceThreadsRoot(vw_net_native::workspace_context* workspace, size_t total, size_t node);
  API void WorkspaceSetUpAllReduceThreadsNode(vw_net_native::workspace_context* workspace, size_t total, size_t node,
      vw_net_native::workspace_context* root_workspace);

  API vw_net_native::ERROR_CODE WorkspaceRunMultiPass(
      vw_net_native::workspace_context* workspace, VW::experimental::api_status* status);
  API vw_net_native::ERROR_CODE WorkspaceNotifyEndOfPass(
      vw_net_native::workspace_context* workspace, VW::experimental::api_status* status);
  API vw_net_native::ERROR_CODE WorkspaceRunDriver(
      vw_net_native::workspace_context* workspace, VW::experimental::api_status* status);

  API vw_net_native::ERROR_CODE WorkspaceParseJson(vw_net_native::workspace_context* workspace, char* json,
      size_t length, vw_net_native::example_pool_get_example_fn get_example, void* example_pool_context,
      VW::experimental::api_status* status);
  API vw_net_native::ERROR_CODE WorkspaceParseDecisionServiceJson(vw_net_native::workspace_context* workspace,
      char* json, size_t length, size_t offset, bool copy_json, vw_net_native::example_pool_get_example_fn get_example,
      void* example_pool_context, VW::parsers::json::decision_service_interaction* interaction,
      VW::experimental::api_status* status);

  API vw_net_native::ERROR_CODE WorkspaceParseSingleLine(vw_net_native::workspace_context* workspace, VW::example* ex,
      char* line, size_t length, VW::experimental::api_status* status);

  API vw_net_native::ERROR_CODE WorkspacePredict(vw_net_native::workspace_context* workspace, VW::example* example,
      vw_net_native::create_prediction_callback_fn, VW::experimental::api_status* status);
  API vw_net_native::ERROR_CODE WorkspaceLearn(vw_net_native::workspace_context* workspace, VW::example* example,
      vw_net_native::create_prediction_callback_fn, VW::experimental::api_status* status);
  API vw_net_native::ERROR_CODE WorkspacePredictMulti(vw_net_native::workspace_context* workspace,
      VW::multi_ex* example, vw_net_native::create_prediction_callback_fn, VW::experimental::api_status* status);
  API vw_net_native::ERROR_CODE WorkspaceLearnMulti(vw_net_native::workspace_context* workspace, VW::multi_ex* example,
      vw_net_native::create_prediction_callback_fn, VW::experimental::api_status* status);

  API char* WorkspaceGetIdDup(vw_net_native::workspace_context* workspace);
  API void WorkspaceSetId(vw_net_native::workspace_context* workspace, char* id, size_t id_length);

  API VW::label_type_t WorkspaceGetLabelType(vw_net_native::workspace_context* workspace);
}
