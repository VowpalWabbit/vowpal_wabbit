#include "ds_ranking_response.h"


namespace decision_service {

	const std::string& ranking_response::uuid() const
	{
		return _uuid;
	}

	int ranking_response::top_action_id() const
	{
		if (_ranking.empty()) {
			throw std::exception();
		}
		return _ranking.front().first;
	}

	const std::vector<std::pair<int, float>>& ranking_response::ranking() const
	{
		return _ranking;
	}

	ranking_response::ranking_response(const std::string & uuid, const std::vector<std::pair<int, float>>& ranking)
		: _uuid(uuid), _ranking(ranking)
	{}

}

