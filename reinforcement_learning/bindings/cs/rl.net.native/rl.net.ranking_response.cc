#include "rl.net.ranking_response.h"

class ranking_enumerator_adapter
{
private:
    reinforcement_learning::ranking_response::const_iterator current;
    reinforcement_learning::ranking_response::const_iterator end;

public:
    inline ranking_enumerator_adapter(const reinforcement_learning::ranking_response* response) : current{response->begin()}, end{response->end()}
    {}

public:
    inline int check_current()
    {
        if (this->current != this->end)
        {
            return 1;
        }

        return 0;
    }

    inline int move_next()
    {
        if ((this->current != this->end) &&
            ((++this->current) != this->end))
        {
            return 1;
        }

        return 0;
    }

    inline reinforcement_learning::action_prob operator*() const
    {
        return *this->current;
    }
};

API reinforcement_learning::ranking_response* CreateRankingResponse()
{
    return new reinforcement_learning::ranking_response();
}

API void DeleteRankingResponse(reinforcement_learning::ranking_response* ranking)
{
    delete ranking;
}

API const char* GetRankingEventId(reinforcement_learning::ranking_response* ranking)
{
    return ranking->get_event_id();
}

API const char* GetRankingModelId(reinforcement_learning::ranking_response* ranking)
{
    return ranking->get_model_id();
}

API size_t GetRankingActionCount(reinforcement_learning::ranking_response* ranking)
{
    return ranking->size();
}

API int GetRankingChosenAction(reinforcement_learning::ranking_response* ranking, size_t* action_id, reinforcement_learning::api_status* status)
{
    return ranking->get_chosen_action_id(*action_id, status);
}

API ranking_enumerator_adapter* CreateRankingEnumeratorAdapter(reinforcement_learning::ranking_response* ranking)
{
    return new ranking_enumerator_adapter(ranking);
}

API void DeleteRankingEnumeratorAdapter(ranking_enumerator_adapter* adapter)
{
    delete adapter;
}

API int RankingEnumeratorInit(ranking_enumerator_adapter* adapter)
{
    return adapter->check_current();
}

API int RankingEnumeratorMoveNext(ranking_enumerator_adapter* adapter)
{
  return adapter->move_next();
}

API reinforcement_learning::action_prob_d GetRankingEnumeratorCurrent(ranking_enumerator_adapter* adapter)
{
    return **adapter;
}