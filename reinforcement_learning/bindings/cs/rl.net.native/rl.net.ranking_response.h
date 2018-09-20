#pragma once

#include "rl.net.native.h"

// TODO: Make the underlying iterator more ammenable to P/Invoke projection
class ranking_enumerator_adapter;

// Global exports
extern "C" {
    // NOTE: THIS IS NOT POLYMORPHISM SAFE!
    API r::ranking_response* CreateRankingResponse();
    API void DeleteRankingResponse(r::ranking_response* ranking);

    // TODO: We should think about how to avoid extra string copies; ideally, err constants
    // should be able to be shared between native/managed, but not clear if this is possible
    // right now.
    API const char* GetRankingEventId(r::ranking_response* ranking);
    API const char* GetRankingModelId(r::ranking_response* ranking);

    // TODO: Would it be more clear to call it GetRankingActionCount, or to keep parallel with
    // rllib nomenclature?
    API size_t GetRankingSize(r::ranking_response* ranking);

    API int GetRankingChosenAction(r::ranking_response* ranking, size_t* action_id, r::api_status* status = nullptr);

    API ranking_enumerator_adapter* CreateRankingEnumeratorAdapter(r::ranking_response* ranking);
    API void DeleteRankingEnumeratorAdapter(ranking_enumerator_adapter* adapter);

    API int RankingEnumeratorMoveNext(ranking_enumerator_adapter* adapter);
    API r::action_prob GetRankingEnumeratorCurrent(ranking_enumerator_adapter* adapter);
}