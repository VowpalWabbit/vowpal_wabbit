#include "vw.net.ds_interaction.h"

API VW::details::DecisionServiceInteraction* CreateDecisionServiceInteraction()
{
  return new VW::details::DecisionServiceInteraction();
}

API void DeleteDecisionServiceInteraction(VW::details::DecisionServiceInteraction* interaction) { delete interaction; }

API char* GetDSInteractionEventIdDup(VW::details::DecisionServiceInteraction* interaction)
{
  return vw_net_native::stdstr_to_cstr(interaction->eventId);
}

API char* GetDSInteractionTimestampDup(VW::details::DecisionServiceInteraction* interaction)
{
  return vw_net_native::stdstr_to_cstr(interaction->timestamp);
}

API size_t GetDSInteractionActionsCount(VW::details::DecisionServiceInteraction* interaction)
{
  return interaction->actions.size();
}

API vw_net_native::dotnet_size_t GetDSInteractionActions(
    VW::details::DecisionServiceInteraction* interaction, unsigned int* actions, vw_net_native::dotnet_size_t count)
{
  return vw_net_native::stdvector_copy_to_managed(interaction->actions, actions, count);
}

API size_t GetDSInteractionProbabilitiesCount(VW::details::DecisionServiceInteraction* interaction)
{
  return interaction->probabilities.size();
}

API vw_net_native::dotnet_size_t GetDSInteractionProbabilities(
    VW::details::DecisionServiceInteraction* interaction, float* probabilities, vw_net_native::dotnet_size_t count)
{
  return vw_net_native::stdvector_copy_to_managed(interaction->probabilities, probabilities, count);
}

API size_t GetDSInteractionBaselineActionsCount(VW::details::DecisionServiceInteraction* interaction)
{
  return interaction->baseline_actions.size();
}

API vw_net_native::dotnet_size_t GetDSInteractionBaselineActions(VW::details::DecisionServiceInteraction* interaction,
    unsigned int* baseline_actions, vw_net_native::dotnet_size_t count)
{
  return vw_net_native::stdvector_copy_to_managed(interaction->baseline_actions, baseline_actions, count);
}

API float GetDSInteractionProbabilityOfDrop(VW::details::DecisionServiceInteraction* interaction)
{
  return interaction->probabilityOfDrop;
}

API float GetDSInteractionOriginalLabelCost(VW::details::DecisionServiceInteraction* interaction)
{
  return interaction->originalLabelCost;
}

API float GetDSInteractionOriginalLabelCostFirstSlot(VW::details::DecisionServiceInteraction* interaction)
{
  return interaction->originalLabelCostFirstSlot;
}

API int GetDSInteractionSkipLearn(VW::details::DecisionServiceInteraction* interaction)
{
  return interaction->skipLearn;
}
