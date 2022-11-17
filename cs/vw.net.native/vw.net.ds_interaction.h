#pragma once

#include "vw.net.native.h"
#include "vw/core/json_utils.h"
#include "vw/core/vw.h"

extern "C"
{
  API VW::details::decision_service_interaction* CreateDecisionServiceInteraction();
  API void DeleteDecisionServiceInteraction(VW::details::decision_service_interaction* interaction);

  API char* GetDSInteractionEventIdDup(VW::details::decision_service_interaction* interaction);
  API char* GetDSInteractionTimestampDup(VW::details::decision_service_interaction* interaction);
  API size_t GetDSInteractionActionsCount(VW::details::decision_service_interaction* interaction);
  API vw_net_native::dotnet_size_t GetDSInteractionActions(VW::details::decision_service_interaction* interaction,
      unsigned int* values, vw_net_native::dotnet_size_t buffer_size);
  API size_t GetDSInteractionProbabilitiesCount(VW::details::decision_service_interaction* interaction);
  API vw_net_native::dotnet_size_t GetDSInteractionProbabilities(
      VW::details::decision_service_interaction* interaction, float* values, vw_net_native::dotnet_size_t buffer_size);
  API size_t GetDSInteractionBaselineActionsCount(VW::details::decision_service_interaction* interaction);
  API vw_net_native::dotnet_size_t GetDSInteractionBaselineActions(
      VW::details::decision_service_interaction* interaction, unsigned int* values,
      vw_net_native::dotnet_size_t buffer_size);

  API float GetDSInteractionProbabilityOfDrop(VW::details::decision_service_interaction* interaction);
  API float GetDSInteractionOriginalLabelCost(VW::details::decision_service_interaction* interaction);
  API float GetDSInteractionOriginalLabelCostFirstSlot(VW::details::decision_service_interaction* interaction);
  API int GetDSInteractionSkipLearn(VW::details::decision_service_interaction* interaction);
}
