#pragma once

#include "vw.net.native.h"
#include "vw/core/vw.h"
#include "vw/core/json_utils.h"

extern "C" {
  API DecisionServiceInteraction* CreateDecisionServiceInteraction();
  API void DeleteDecisionServiceInteraction(DecisionServiceInteraction* interaction);

  API char* GetDSInteractionEventIdDup(DecisionServiceInteraction* interaction);
  API char* GetDSInteractionTimestampDup(DecisionServiceInteraction* interaction);
  API size_t GetDSInteractionActionsCount(DecisionServiceInteraction* interaction);
  API vw_net_native::dotnet_size_t GetDSInteractionActions(DecisionServiceInteraction* interaction, unsigned int* values, vw_net_native::dotnet_size_t buffer_size);
  API size_t GetDSInteractionProbabilitiesCount(DecisionServiceInteraction* interaction);
  API vw_net_native::dotnet_size_t GetDSInteractionProbabilities(DecisionServiceInteraction* interaction, float* values, vw_net_native::dotnet_size_t buffer_size);
  API size_t GetDSInteractionBaselineActionsCount(DecisionServiceInteraction* interaction);
  API vw_net_native::dotnet_size_t GetDSInteractionBaselineActions(DecisionServiceInteraction* interaction, unsigned int* values, vw_net_native::dotnet_size_t buffer_size);

  API float GetDSInteractionProbabilityOfDrop(DecisionServiceInteraction* interaction);
  API float GetDSInteractionOriginalLabelCost(DecisionServiceInteraction* interaction);
  API float GetDSInteractionOriginalLabelCostFirstSlot(DecisionServiceInteraction* interaction);
  API int GetDSInteractionSkipLearn(DecisionServiceInteraction* interaction);
}
