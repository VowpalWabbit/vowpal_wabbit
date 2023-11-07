#include "vw.net.arguments.h"

#include "vw/config/cli_options_serializer.h"
#include "vw/config/options.h"

API void GetWorkspaceBasicArguments(
    vw_net_native::workspace_context* workspace, vw_net_native::vw_basic_arguments_t* args)
{
  args->is_test_only = !workspace->vw->runtime_config.training;
  args->num_passes = (int)workspace->vw->runtime_config.numpasses;
  args->learning_rate = workspace->vw->update_rule_config.eta;
  args->power_t = workspace->vw->update_rule_config.power_t;

  if (workspace->vw->options->was_supplied("cb"))
  {
    args->cb_number_of_actions = (int)workspace->vw->options->get_typed_option<uint32_t>("cb").value();
  }
}

API const char* GetWorkspaceDataFilename(vw_net_native::workspace_context* workspace)
{
  return workspace->vw->parser_runtime.data_filename.c_str();
}

API const char* GetFinalRegressorFilename(vw_net_native::workspace_context* workspace)
{
  return workspace->vw->output_model_config.final_regressor_name.c_str();
}

API char* SerializeCommandLine(vw_net_native::workspace_context* workspace)
{
  VW::config::options_i* options = workspace->vw->options.get();
  VW::config::cli_options_serializer serializer;
  for (auto const& option : options->get_all_options())
  {
    if (options->was_supplied(option->m_name)) { serializer.add(*option); }
  }

  auto serialized_keep_options = serializer.str();
  return strdup(serialized_keep_options.c_str());
}

API size_t GetInitialRegressorFilenamesCount(vw_net_native::workspace_context* workspace)
{
  return workspace->vw->initial_weights_config.initial_regressors.size();
}

API vw_net_native::dotnet_size_t GetInitialRegressorFilenames(
    vw_net_native::workspace_context* workspace, const char** filenames, vw_net_native::dotnet_size_t count)
{
  std::vector<std::string>& initial_regressors = workspace->vw->initial_weights_config.initial_regressors;
  size_t size = initial_regressors.size();
  if ((size_t)count < size)
  {
    return vw_net_native::size_to_neg_dotnet_size(size);  // Not enough space in destination buffer
  }

  for (size_t i = 0; i < size; i++)
  {
    filenames[i] = workspace->vw->initial_weights_config.initial_regressors[i].c_str();
  }

  return workspace->vw->initial_weights_config.initial_regressors.size();
}
