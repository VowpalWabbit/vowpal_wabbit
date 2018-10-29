#include "rl.net.live_model.h"

static void pipe_managed_callback(const reinforcement_learning::api_status& status, livemodel_context_t* context)
{
    auto managed_callback_local = context->callback;
    if (managed_callback_local)
    {
        managed_callback_local(status);
    }
}

API livemodel_context_t* CreateLiveModel(reinforcement_learning::utility::configuration* config)
{
    livemodel_context_t* context = new livemodel_context_t;
    context->callback = nullptr;

    context->livemodel = new reinforcement_learning::live_model(*config, pipe_managed_callback, context);

    return context;
}

API void DeleteLiveModel(livemodel_context_t* context)
{
    // Since the livemodel destructor waits for queues to drain, this can have unhappy consequences,
    // so detach the callback pipe first. This will cause all background callbacks to no-op in the
    // unmanaged side, which maintains expected thread semantics (the user of the bindings)
    context->callback = nullptr;

    delete context->livemodel;
    delete context;
}

API int LiveModelInit(livemodel_context_t* context, reinforcement_learning::api_status* status)
{
    return context->livemodel->init(status);
}

API int LiveModelChooseRank(livemodel_context_t* context, const char * event_id, const char * context_json, reinforcement_learning::ranking_response* resp, reinforcement_learning::api_status* status)
{
    return context->livemodel->choose_rank(event_id, context_json, *resp, status);
}

API int LiveModelReportOutcomeF(livemodel_context_t* context, const char * event_id, float outcome, reinforcement_learning::api_status* status)
{
    return context->livemodel->report_outcome(event_id, outcome, status);
}

API int LiveModelReportOutcomeJson(livemodel_context_t* context, const char * event_id, const char * outcomeJson, reinforcement_learning::api_status* status)
{
    return context->livemodel->report_outcome(event_id, outcomeJson, status);
}

API void LiveModelSetCallback(livemodel_context_t* livemodel, managed_callback_t callback)
{
    livemodel->callback = callback;
}
