#include "vw.net.workspace.h"
#include "vw/json_parser/parse_example_json.h"

#include <functional>

API vw_net_native::ERROR_CODE WorkspaceParseJson(vw_net_native::workspace_context* workspace, char* json, size_t length,
    vw_net_native::example_pool_get_example_fn get_example, void* example_pool_context,
    VW::experimental::api_status* status)
{
  VW::multi_ex examples;

  auto& ex = get_example(example_pool_context);
  examples.push_back(&ex);

  try
  {
    if (workspace->vw->output_config.audit)
    {
      VW::parsers::json::read_line_json<true>(
          *workspace->vw, examples, json, length, std::bind(get_example, example_pool_context));
    }
    else
    {
      VW::parsers::json::read_line_json<false>(
          *workspace->vw, examples, json, length, std::bind(get_example, example_pool_context));
    }

    VW::setup_examples(*workspace->vw, examples);

    // delete native array of pointers, keep examples (they are owned by the managed-side)
    examples.clear();

    return VW::experimental::error_code::success;
  }
  CATCH_RETURN_STATUS
}

API vw_net_native::ERROR_CODE WorkspaceParseDecisionServiceJson(vw_net_native::workspace_context* workspace, char* json,
    size_t length, size_t offset, bool copy_json, vw_net_native::example_pool_get_example_fn get_example,
    void* example_pool_context, VW::parsers::json::decision_service_interaction* interaction,
    VW::experimental::api_status* status)
{
  char* actual_json = json + offset;
  VW::multi_ex examples;

  auto& ex = get_example(example_pool_context);
  examples.push_back(&ex);

  try
  {
    if (workspace->vw->output_config.audit)
    {
      VW::parsers::json::read_line_decision_service_json<true>(*workspace->vw, examples, actual_json, length, copy_json,
          std::bind(get_example, example_pool_context), interaction);
    }
    else
    {
      VW::parsers::json::read_line_decision_service_json<false>(*workspace->vw, examples, actual_json, length,
          copy_json, std::bind(get_example, example_pool_context), interaction);
    }

    VW::setup_examples(*workspace->vw, examples);

    // delete native array of pointers, keep examples (they are owned by the managed-side)
    examples.clear();

    return VW::experimental::error_code::success;
  }
  CATCH_RETURN_STATUS
}
