#pragma once

#include "rl.net.native.h"

typedef void (*managed_callback_t)(const reinforcement_learning::api_status&);

typedef struct livemodel_context {
    reinforcement_learning::live_model* livemodel;
    managed_callback_t callback;
} livemodel_context_t;

// Global exports
extern "C" {
    // NOTE: THIS IS NOT POLYMORPHISM SAFE!
    API livemodel_context_t* CreateLiveModel(reinforcement_learning::utility::configuration* config);
    API void DeleteLiveModel(livemodel_context_t* livemodel);

    API int LiveModelInit(livemodel_context_t* livemodel, reinforcement_learning::api_status* status = nullptr);
    API int LiveModelChooseRank(livemodel_context_t* livemodel, const char * event_id, const char * context_json, reinforcement_learning::ranking_response* resp, reinforcement_learning::api_status* status = nullptr);
    API int LiveModelReportOutcomeF(livemodel_context_t* livemodel, const char * event_id, float outcome, reinforcement_learning::api_status* status = nullptr);
    API int LiveModelReportOutcomeJson(livemodel_context_t* livemodel, const char * event_id, const char * outcomeJson, reinforcement_learning::api_status* status = nullptr);

    API void LiveModelSetCallback(livemodel_context_t* livemodel, managed_callback_t callback = nullptr);
}
