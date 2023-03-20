#include "vw.net.workspace.h"

#include "vw/allreduce/allreduce.h"
#include "vw/common/string_view.h"
#include "vw/core/best_constant.h"
#include "vw/core/learner.h"
#include "vw/core/shared_data.h"
#include "vw/text_parser/parse_example_text.h"

vw_net_native::workspace_context* create_workspace(
    std::string arguments, VW::io_buf* model, trace_message_t trace_listener, void* trace_context)
{
  vw_net_native::workspace_context* context = new vw_net_native::workspace_context();
  context->vw = VW::initialize(arguments, model, false, trace_listener, trace_context);
  context->trace_listener = trace_listener;
  context->trace_listener_context = trace_context;

  // TODO: We are maybe leaking the model_reader here - similar to how we seem
  // to leak the Stream adapter in the old C#/CLI codebase. See vw_base.cpp.

  return context;
}

vw_net_native::workspace_context* create_workspace_from_seed(
    VW::workspace* seed, std::string extra_arguments, trace_message_t trace_listener, void* trace_context)
{
  vw_net_native::workspace_context* context = new vw_net_native::workspace_context();
  context->vw = VW::seed_vw_model(seed, extra_arguments, trace_listener, trace_context);
  context->trace_listener = trace_listener;
  context->trace_listener_context = trace_context;

  // TODO: We are maybe leaking the model_reader here - similar to how we seem
  // to leak the Stream adapter in the old C#/CLI codebase. See vw_base.cpp.

  return context;
}

API vw_net_native::workspace_context* CreateWorkspaceWithSeedVwModel(vw_net_native::workspace_context* seed,
    char* arguments, size_t arguments_size, trace_message_t trace_listener, void* trace_context,
    VW::experimental::api_status* status)
{
  try
  {
    std::string arguments_str(arguments, arguments_size);

    return create_workspace_from_seed(seed->vw, arguments_str, trace_listener, trace_context);
  }
  CATCH_FILL_STATUS

  return nullptr;
}

API vw_net_native::workspace_context* CreateWorkspaceWithModelData(char* arguments, size_t arguments_size,
    vw_net_native::io_reader_vtable model_reader_vtable, trace_message_t trace_listener, void* trace_context,
    VW::experimental::api_status* status)
{
  try
  {
    VW::io_buf model;
    model.add_file(std::unique_ptr<VW::io::reader>(new vw_net_native::stream_io_reader(model_reader_vtable)));

    std::string arguments_str(arguments, arguments_size);

    return create_workspace(arguments_str, &model, trace_listener, trace_context);
  }
  CATCH_FILL_STATUS

  return nullptr;
}

API vw_net_native::workspace_context* CreateWorkspace(char* arguments, size_t arguments_size,
    trace_message_t trace_listener, void* trace_context, VW::experimental::api_status* status)
{
  try
  {
    std::string arguments_str(arguments, arguments_size);
    return create_workspace(arguments_str, nullptr, trace_listener, trace_context);
  }
  CATCH_FILL_STATUS

  return nullptr;
}

API vw_net_native::ERROR_CODE DeleteWorkspace(
    vw_net_native::workspace_context* workspace, VW::experimental::api_status* status)
{
  try
  {
    VW::finish(*workspace->vw);
    return VW::experimental::error_code::success;
  }
  CATCH_RETURN_STATUS
}

API VW::prediction_type_t WorkspaceGetOutputPredictionType(vw_net_native::workspace_context* workspace)
{
  return workspace->vw->l->get_output_prediction_type();
}

API vw_net_native::ERROR_CODE WorkspaceReload(vw_net_native::workspace_context* workspace, char* arguments,
    size_t arguments_size, VW::experimental::api_status* status)
{
  try
  {
    std::string arguments_str(arguments, arguments_size);
    VW::details::reset_source(*workspace->vw, workspace->vw->initial_weights_config.num_bits);

    auto buffer = std::make_shared<std::vector<char>>();
    {
      VW::io_buf write_buffer;
      write_buffer.add_file(VW::io::create_vector_writer(buffer));
      VW::save_predictor(*workspace->vw, write_buffer);
    }

    // make sure don't try to free m_vw twice in case VW::finish throws.
    VW::workspace* vw_tmp = nullptr;
    std::swap(workspace->vw, vw_tmp);
    VW::finish(*vw_tmp);

    // reload from model
    // seek to beginning
    VW::io_buf reader_view_of_buffer;
    reader_view_of_buffer.add_file(VW::io::create_buffer_view(buffer->data(), buffer->size()));

    workspace->vw = VW::initialize(arguments_str, &reader_view_of_buffer);

    return VW::experimental::error_code::success;
  }
  CATCH_RETURN_STATUS
}

API vw_net_native::ERROR_CODE WorkspaceSavePredictorToFile(vw_net_native::workspace_context* workspace, char* file_name,
    size_t file_name_size, VW::experimental::api_status* status)
{
  try
  {
    std::string file_name_str(file_name, file_name_size);
    VW::save_predictor(*workspace->vw, file_name_str);

    return VW::experimental::error_code::success;
  }
  CATCH_RETURN_STATUS
}

API vw_net_native::ERROR_CODE WorkspaceSavePredictorToWriter(vw_net_native::workspace_context* workspace,
    vw_net_native::io_writer_vtable writer_vtable, VW::experimental::api_status* status)
{
  try
  {
    VW::io_buf write_buffer;
    write_buffer.add_file(std::unique_ptr<VW::io::writer>(new vw_net_native::stream_io_writer(writer_vtable)));
    VW::save_predictor(*workspace->vw, write_buffer);

    return VW::experimental::error_code::success;
  }
  CATCH_RETURN_STATUS
}

API void WorkspaceGetPerformanceStatistics(
    vw_net_native::workspace_context* workspace, vw_net_native::performance_statistics_t* statistics)
{
  if (workspace->vw->passes_config.current_pass == 0)
  {
    statistics->examples_per_pass = workspace->vw->sd->example_number;
  }
  else
  {
    statistics->examples_per_pass = workspace->vw->sd->example_number / workspace->vw->passes_config.current_pass;
  }

  statistics->weighted_examples = workspace->vw->sd->weighted_examples();
  statistics->weighted_labels = workspace->vw->sd->weighted_labels;

  if (workspace->vw->passes_config.holdout_set_off)
  {
    if (workspace->vw->sd->weighted_labeled_examples > 0)
    {
      statistics->average_loss = workspace->vw->sd->sum_loss / workspace->vw->sd->weighted_labeled_examples;
    }
    else { statistics->average_loss = NAN; }
  }
  else if ((workspace->vw->sd->holdout_best_loss == FLT_MAX) || (workspace->vw->sd->holdout_best_loss == FLT_MAX * 0.5))
  {
    statistics->average_loss = NAN;
  }
  else { statistics->average_loss = workspace->vw->sd->holdout_best_loss; }

  float best_constant;
  float best_constant_loss;
  if (get_best_constant(*workspace->vw->loss_config.loss.get(), *workspace->vw->sd, best_constant, best_constant_loss))
  {
    statistics->best_constant = best_constant;
    if (best_constant_loss != FLT_MIN) { statistics->best_constant_loss = best_constant_loss; }
  }

  statistics->total_features = workspace->vw->sd->total_features;
}

API size_t WorkspaceHashSpace(vw_net_native::workspace_context* workspace, char* space, size_t space_size)
{
  std::string space_str(space, space_size);
  return VW::hash_space(*workspace->vw, space_str);
}

API size_t WorkspaceHashFeature(
    vw_net_native::workspace_context* workspace, char* feature, size_t feature_size, size_t space_hash)
{
  std::string feature_str(feature, feature_size);
  return VW::hash_feature(*workspace->vw, feature_str, space_hash);
}

API void WorkspaceSetUpAllReduceThreadsRoot(vw_net_native::workspace_context* workspace, size_t total, size_t node)
{
  workspace->vw->runtime_config.selected_all_reduce_type = VW::all_reduce_type::THREAD;
  workspace->vw->runtime_state.all_reduce.reset(new VW::all_reduce_threads(total, node));
}

API void WorkspaceSetUpAllReduceThreadsNode(vw_net_native::workspace_context* workspace, size_t total, size_t node,
    vw_net_native::workspace_context* root_workspace)
{
  workspace->vw->runtime_config.selected_all_reduce_type = VW::all_reduce_type::THREAD;
  workspace->vw->runtime_state.all_reduce.reset(new VW::all_reduce_threads(
      (VW::all_reduce_threads*)root_workspace->vw->runtime_state.all_reduce.get(), total, node));
}

API vw_net_native::ERROR_CODE WorkspaceRunMultiPass(
    vw_net_native::workspace_context* workspace, VW::experimental::api_status* status)
{
  if (workspace->vw->runtime_config.numpasses > 1)
  {
    try
    {
      workspace->vw->runtime_state.do_reset_source = true;
      VW::start_parser(*workspace->vw);
      VW::LEARNER::generic_driver(*workspace->vw);
      VW::end_parser(*workspace->vw);
    }
    CATCH_RETURN_STATUS
  }

  // In .NET Framework implementation, in the case where numpasses is 1, we silently NoOp
  // here, so for now just return success, regardless.
  return VW::experimental::error_code::success;
}

API vw_net_native::ERROR_CODE WorkspaceNotifyEndOfPass(
    vw_net_native::workspace_context* workspace, VW::experimental::api_status* status)
{
  try
  {
    workspace->vw->l->end_pass();
    sync_stats(*workspace->vw);

    return VW::experimental::error_code::success;
  }
  CATCH_RETURN_STATUS
}

API vw_net_native::ERROR_CODE WorkspaceParseSingleLine(vw_net_native::workspace_context* workspace, VW::example* ex,
    char* line, size_t length, VW::experimental::api_status* status)
{
  try
  {
    VW::parsers::text::read_line(*workspace->vw, ex, VW::string_view(line, length));

    VW::setup_example(*workspace->vw, ex);

    return VW::experimental::error_code::success;
  }
  CATCH_RETURN_STATUS
}

API vw_net_native::ERROR_CODE WorkspacePredict(vw_net_native::workspace_context* workspace, VW::example* ex,
    vw_net_native::create_prediction_callback_fn create_prediction, VW::experimental::api_status* status)
{
  try
  {
    require_singleline(workspace->vw->l)->predict(*ex);

    if (create_prediction != nullptr) { create_prediction(); }

    require_singleline(workspace->vw->l)->finish_example(*workspace->vw, *ex);

    return VW::experimental::error_code::success;
  }
  CATCH_RETURN_STATUS
}

API vw_net_native::ERROR_CODE WorkspaceLearn(vw_net_native::workspace_context* workspace, VW::example* ex,
    vw_net_native::create_prediction_callback_fn create_prediction, VW::experimental::api_status* status)
{
  try
  {
    workspace->vw->learn(*ex);

    if (create_prediction != nullptr) { create_prediction(); }

    require_singleline(workspace->vw->l)->finish_example(*workspace->vw, *ex);

    return VW::experimental::error_code::success;
  }
  CATCH_RETURN_STATUS
}

API vw_net_native::ERROR_CODE WorkspacePredictMulti(vw_net_native::workspace_context* workspace, VW::multi_ex* ex_coll,
    vw_net_native::create_prediction_callback_fn create_prediction, VW::experimental::api_status* status)
{
  try
  {
    require_multiline(workspace->vw->l)->predict(*ex_coll);

    if (create_prediction != nullptr) { create_prediction(); }

    require_multiline(workspace->vw->l)->finish_example(*workspace->vw, *ex_coll);

    return VW::experimental::error_code::success;
  }
  CATCH_RETURN_STATUS
}

API vw_net_native::ERROR_CODE WorkspaceLearnMulti(vw_net_native::workspace_context* workspace, VW::multi_ex* ex_coll,
    vw_net_native::create_prediction_callback_fn create_prediction, VW::experimental::api_status* status)
{
  try
  {
    workspace->vw->learn(*ex_coll);

    if (create_prediction != nullptr) { create_prediction(); }

    require_multiline(workspace->vw->l)->finish_example(*workspace->vw, *ex_coll);

    return VW::experimental::error_code::success;
  }
  CATCH_RETURN_STATUS
}

API char* WorkspaceGetIdDup(vw_net_native::workspace_context* workspace)
{
  return vw_net_native::stdstr_to_cstr(workspace->vw->id);
}

API void WorkspaceSetId(vw_net_native::workspace_context* workspace, char* id, size_t id_length)
{
  workspace->vw->id = std::string(id, id_length);
}

API VW::label_type_t WorkspaceGetLabelType(vw_net_native::workspace_context* workspace)
{
  return workspace->vw->parser_runtime.example_parser->lbl_parser.label_type;
}
