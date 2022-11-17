#pragma once

#include "vw.net.native.h"
#include "vw.net.workspace.h"

namespace vw_net_native
{
struct vw_basic_arguments_t
{
  dotnet_bool_u1_t is_test_only;
  int num_passes;
  int cb_number_of_actions;
  float learning_rate;
  float power_t;
};
}  // namespace vw_net_native

extern "C"
{
  API void GetWorkspaceBasicArguments(
      vw_net_native::workspace_context* workspace, vw_net_native::vw_basic_arguments_t* args);
  API const char* GetWorkspaceDataFilename(vw_net_native::workspace_context* workspace);
  API const char* GetFinalRegressorFilename(vw_net_native::workspace_context* workspace);
  API char* SerializeCommandLine(vw_net_native::workspace_context* workspace);
  API size_t GetInitialRegressorFilenamesCount(vw_net_native::workspace_context* workspace);
  API vw_net_native::dotnet_size_t GetInitialRegressorFilenames(
      vw_net_native::workspace_context* workspace, const char** filenames, vw_net_native::dotnet_size_t count);
}
