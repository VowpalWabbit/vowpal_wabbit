#include "vw.net.ds_interaction.h"

API DecisionServiceInteraction* CreateDecisionServiceInteraction()
{
  return new DecisionServiceInteraction();
}

API void DeleteDecisionServiceInteraction(DecisionServiceInteraction* interaction)
{
  delete interaction;
}

API char* GetDSInteractionEventIdDup(DecisionServiceInteraction* interaction)
{
  return vw_net_native::stdstr_to_cstr(interaction->eventId);
}

API char* GetDSInteractionTimestampDup(DecisionServiceInteraction* interaction)
{
  return vw_net_native::stdstr_to_cstr(interaction->timestamp);
}

API size_t GetDSInteractionActionsCount(DecisionServiceInteraction* interaction)
{
  return interaction->actions.size();
}

API vw_net_native::dotnet_size_t GetDSInteractionActions(DecisionServiceInteraction* interaction, unsigned int* actions, vw_net_native::dotnet_size_t count)
{
  return vw_net_native::stdvector_copy_to_managed(interaction->actions, actions, count);
}

API size_t GetDSInteractionProbabilitiesCount(DecisionServiceInteraction* interaction)
{
  return interaction->probabilities.size();
}

API vw_net_native::dotnet_size_t GetDSInteractionProbabilities(DecisionServiceInteraction* interaction, float* probabilities, vw_net_native::dotnet_size_t count)
{
  return vw_net_native::stdvector_copy_to_managed(interaction->probabilities, probabilities, count);
}

API size_t GetDSInteractionBaselineActionsCount(DecisionServiceInteraction* interaction)
{
  return interaction->baseline_actions.size();
}

API vw_net_native::dotnet_size_t GetDSInteractionBaselineActions(DecisionServiceInteraction* interaction, unsigned int* baseline_actions, vw_net_native::dotnet_size_t count)
{
  return vw_net_native::stdvector_copy_to_managed(interaction->baseline_actions, baseline_actions, count);
}

API float GetDSInteractionProbabilityOfDrop(DecisionServiceInteraction* interaction)
{
  return interaction->probabilityOfDrop;
}

API float GetDSInteractionOriginalLabelCost(DecisionServiceInteraction* interaction)
{
  return interaction->originalLabelCost;
}

API float GetDSInteractionOriginalLabelCostFirstSlot(DecisionServiceInteraction* interaction)
{
  return interaction->originalLabelCostFirstSlot;
}

API int GetDSInteractionSkipLearn(DecisionServiceInteraction* interaction)
{
  return interaction->skipLearn;
}
