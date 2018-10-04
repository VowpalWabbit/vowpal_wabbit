#include "rl.net.live_model.h"

API reinforcement_learning::live_model* CreateLiveModel(reinforcement_learning::utility::configuration* config)
{
    return new reinforcement_learning::live_model(*config);
}

API void DeleteLiveModel(reinforcement_learning::live_model* livemodel)
{
    delete livemodel;
}

API int LiveModelInit(reinforcement_learning::live_model* livemodel, reinforcement_learning::api_status* status)
{
    return livemodel->init(status);
}

API int LiveModelChooseRank(reinforcement_learning::live_model* livemodel, const char * event_id, const char * context_json, reinforcement_learning::ranking_response* resp, reinforcement_learning::api_status* status)
{
    return livemodel->choose_rank(event_id, context_json, *resp, status);
}

API int LiveModelReportOutcome(reinforcement_learning::live_model* livemodel, const char * event_id, float outcome, reinforcement_learning::api_status* status)
{
    return livemodel->report_outcome(event_id, outcome, status);
}