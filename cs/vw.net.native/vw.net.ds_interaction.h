#pragma once

#include "vw.net.native.h"
#include "vw/core/json_utils.h"
#include "vw/core/vw.h"

extern "C"
{
  API VW::details::DecisionServiceInteraction* CreateDecisionServiceInteraction();
  API void DeleteDecisionServiceInteraction(VW::details::DecisionServiceInteraction* interaction);

  API char* GetDSInteractionEventIdDup(VW::details::DecisionServiceInteraction* interaction);
  API char* GetDSInteractionTimestampDup(VW::details::DecisionServiceInteraction* interaction);
  API size_t GetDSInteractionActionsCount(VW::details::DecisionServiceInteraction* interaction);
  API vw_net_native::dotnet_size_t GetDSInteractionActions(VW::details::DecisionServiceInteraction* interaction,
      unsigned int* values, vw_net_native::dotnet_size_t buffer_size);
  API size_t GetDSInteractionProbabilitiesCount(VW::details::DecisionServiceInteraction* interaction);
  API vw_net_native::dotnet_size_t GetDSInteractionProbabilities(
      VW::details::DecisionServiceInteraction* interaction, float* values, vw_net_native::dotnet_size_t buffer_size);
  API size_t GetDSInteractionBaselineActionsCount(VW::details::DecisionServiceInteraction* interaction);
  API vw_net_native::dotnet_size_t GetDSInteractionBaselineActions(VW::details::DecisionServiceInteraction* interaction,
      unsigned int* values, vw_net_native::dotnet_size_t buffer_size);

  API float GetDSInteractionProbabilityOfDrop(VW::details::DecisionServiceInteraction* interaction);
  API float GetDSInteractionOriginalLabelCost(VW::details::DecisionServiceInteraction* interaction);
  API float GetDSInteractionOriginalLabelCostFirstSlot(VW::details::DecisionServiceInteraction* interaction);
  API int GetDSInteractionSkipLearn(VW::details::DecisionServiceInteraction* interaction);
}
