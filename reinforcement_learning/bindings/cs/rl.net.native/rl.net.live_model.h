#pragma once

#include "rl.net.native.h"

// Global exports
extern "C" {
    // NOTE: THIS IS NOT POLYMORPHISM SAFE!
    API reinforcement_learning::live_model* CreateLiveModel(reinforcement_learning::utility::configuration* config);
    API void DeleteLiveModel(reinforcement_learning::live_model* livemodel);

    API int InitLiveModel(reinforcement_learning::live_model* livemodel, reinforcement_learning::api_status* status = nullptr);
    API int LiveModelChooseRank(reinforcement_learning::live_model* livemodel, const char * event_id, const char * context_json, reinforcement_learning::ranking_response* resp, reinforcement_learning::api_status* status = nullptr);
    API int LiveModelReportOutcome(reinforcement_learning::live_model* livemodel, const char * event_id, float outcome, reinforcement_learning::api_status* status = nullptr);
}