#include "rl.net.ranking_response.h"

class ranking_enumerator_adapter
{
private:
    r::ranking_response::ranking_iterator current;
    r::ranking_response::ranking_iterator end;

public:
    inline ranking_enumerator_adapter(r::ranking_response* response) : current{response->begin()}, end{response->end()}
    {}

public:
    inline int move_next()
    {
        if ((this->current != this->end) &&
            ((++this->current) != this->end))
        {
            return 1;
        }

        return 0;
    }

    inline r::action_prob operator*() const
    {
        return *this->current;
    }
};

API r::ranking_response* CreateRankingResponse()
{
    return new r::ranking_response();
}

API void DeleteRankingResponse(r::ranking_response* ranking)
{
    delete ranking;
}

API const char* GetRankingEventId(r::ranking_response* ranking)
{
    return ranking->get_event_id();
}

API const char* GetRankingModelId(r::ranking_response* ranking)
{
    return ranking->get_model_id();
}

API size_t GetRankingSize(r::ranking_response* ranking)
{
    return ranking->size();
}

API int GetRankingChosenAction(r::ranking_response* ranking, size_t* action_id, r::api_status* status)
{
    return ranking->get_chosen_action_id(*action_id, status);
}

API ranking_enumerator_adapter* CreateRankingEnumeratorAdapter(r::ranking_response* ranking)
{
    return new ranking_enumerator_adapter(ranking);
}

API void DeleteRankingEnumeratorAdapter(ranking_enumerator_adapter* adapter)
{
    delete adapter;
}

API int RankingEnumeratorMoveNext(ranking_enumerator_adapter* adapter)
{
    return adapter->move_next();
}

API r::action_prob GetRankingEnumeratorCurrent(ranking_enumerator_adapter* adapter)
{
    return **adapter;
}