#include "vw.net.ds_interaction.h"

API VW::parsers::json::decision_service_interaction* CreateDecisionServiceInteraction()
{
  return new VW::parsers::json::decision_service_interaction();
}

API void DeleteDecisionServiceInteraction(VW::parsers::json::decision_service_interaction* interaction)
{
  delete interaction;
}

API char* GetDSInteractionEventIdDup(VW::parsers::json::decision_service_interaction* interaction)
{
  return vw_net_native::stdstr_to_cstr(interaction->event_id);
}

API char* GetDSInteractionTimestampDup(VW::parsers::json::decision_service_interaction* interaction)
{
  return vw_net_native::stdstr_to_cstr(interaction->timestamp);
}

API size_t GetDSInteractionActionsCount(VW::parsers::json::decision_service_interaction* interaction)
{
  return interaction->actions.size();
}

API vw_net_native::dotnet_size_t GetDSInteractionActions(VW::parsers::json::decision_service_interaction* interaction,
    unsigned int* actions, vw_net_native::dotnet_size_t count)
{
  return vw_net_native::stdvector_copy_to_managed(interaction->actions, actions, count);
}

API size_t GetDSInteractionProbabilitiesCount(VW::parsers::json::decision_service_interaction* interaction)
{
  return interaction->probabilities.size();
}

API vw_net_native::dotnet_size_t GetDSInteractionProbabilities(
    VW::parsers::json::decision_service_interaction* interaction, float* probabilities,
    vw_net_native::dotnet_size_t count)
{
  return vw_net_native::stdvector_copy_to_managed(interaction->probabilities, probabilities, count);
}

API size_t GetDSInteractionBaselineActionsCount(VW::parsers::json::decision_service_interaction* interaction)
{
  return interaction->baseline_actions.size();
}

API vw_net_native::dotnet_size_t GetDSInteractionBaselineActions(
    VW::parsers::json::decision_service_interaction* interaction, unsigned int* baseline_actions,
    vw_net_native::dotnet_size_t count)
{
  return vw_net_native::stdvector_copy_to_managed(interaction->baseline_actions, baseline_actions, count);
}

API float GetDSInteractionProbabilityOfDrop(VW::parsers::json::decision_service_interaction* interaction)
{
  return interaction->probability_of_drop;
}

API float GetDSInteractionOriginalLabelCost(VW::parsers::json::decision_service_interaction* interaction)
{
  return interaction->original_label_cost;
}

API float GetDSInteractionOriginalLabelCostFirstSlot(VW::parsers::json::decision_service_interaction* interaction)
{
  return interaction->original_label_cost_first_slot;
}

API int GetDSInteractionSkipLearn(VW::parsers::json::decision_service_interaction* interaction)
{
  return interaction->skip_learn;
}
