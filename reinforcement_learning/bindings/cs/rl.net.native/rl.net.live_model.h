#pragma once

#include "rl.net.native.h"

// Global exports
extern "C" {
    // NOTE: THIS IS NOT POLYMORPHISM SAFE!
    API r::live_model* CreateLiveModel(u::configuration* config);
    API void DeleteLiveModel(r::live_model* livemodel);

    API int InitLiveModel(r::live_model* livemodel, r::api_status* status = nullptr);
    API int LiveModelChooseRank(r::live_model* livemodel, const char * event_id, const char * context_json, r::ranking_response* resp, r::api_status* status = nullptr);
    API int LiveModelReportOutcome(r::live_model* livemodel, const char * event_id, float outcome, r::api_status* status = nullptr);
}