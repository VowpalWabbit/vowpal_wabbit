#include "rl.net.live_model.h"

API r::live_model* CreateLiveModel(u::configuration* config)
{
    return new r::live_model(*config);
}

API void DeleteLiveModel(r::live_model* livemodel)
{
    delete livemodel;
}

API int InitLiveModel(r::live_model* livemodel, r::api_status* status)
{
    return livemodel->init(status);
}

API int LiveModelChooseRank(r::live_model* livemodel, const char * event_id, const char * context_json, r::ranking_response* resp, r::api_status* status)
{
    return livemodel->choose_rank(event_id, context_json, *resp, status);
}

API int LiveModelReportOutcome(r::live_model* livemodel, const char * event_id, float outcome, r::api_status* status)
{
    return livemodel->report_outcome(event_id, outcome, status);
}