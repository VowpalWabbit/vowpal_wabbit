#pragma once

#include "vw.net.native.h"
#include "vw/core/vw.h"
#include "vw/json_parser/decision_service_utils.h"

extern "C"
{
  API VW::parsers::json::decision_service_interaction* CreateDecisionServiceInteraction();
  API void DeleteDecisionServiceInteraction(VW::parsers::json::decision_service_interaction* interaction);

  API char* GetDSInteractionEventIdDup(VW::parsers::json::decision_service_interaction* interaction);
  API char* GetDSInteractionTimestampDup(VW::parsers::json::decision_service_interaction* interaction);
  API size_t GetDSInteractionActionsCount(VW::parsers::json::decision_service_interaction* interaction);
  API vw_net_native::dotnet_size_t GetDSInteractionActions(VW::parsers::json::decision_service_interaction* interaction,
      unsigned int* values, vw_net_native::dotnet_size_t buffer_size);
  API size_t GetDSInteractionProbabilitiesCount(VW::parsers::json::decision_service_interaction* interaction);
  API vw_net_native::dotnet_size_t GetDSInteractionProbabilities(
      VW::parsers::json::decision_service_interaction* interaction, float* values,
      vw_net_native::dotnet_size_t buffer_size);
  API size_t GetDSInteractionBaselineActionsCount(VW::parsers::json::decision_service_interaction* interaction);
  API vw_net_native::dotnet_size_t GetDSInteractionBaselineActions(
      VW::parsers::json::decision_service_interaction* interaction, unsigned int* values,
      vw_net_native::dotnet_size_t buffer_size);

  API float GetDSInteractionProbabilityOfDrop(VW::parsers::json::decision_service_interaction* interaction);
  API float GetDSInteractionOriginalLabelCost(VW::parsers::json::decision_service_interaction* interaction);
  API float GetDSInteractionOriginalLabelCostFirstSlot(VW::parsers::json::decision_service_interaction* interaction);
  API int GetDSInteractionSkipLearn(VW::parsers::json::decision_service_interaction* interaction);
}
