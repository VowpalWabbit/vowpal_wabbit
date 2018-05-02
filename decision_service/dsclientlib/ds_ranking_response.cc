#include "ds_ranking_response.h"


namespace decision_service {

	const std::string& ranking_response::get_uuid() const
	{
		return _uuid;
	}

	int ranking_response::get_top_action_id() const
	{
		if (_ranking.empty()) {
			throw std::exception();
		}
		return _ranking.front().first;
	}

	const std::vector<std::pair<int, float>>& ranking_response::get_ranking() const
	{
		return _ranking;
	}

	void ranking_response::set_uuid(const std::string & uuid)
	{
		_uuid = uuid;
	}

	void ranking_response::set_ranking(const std::vector<std::pair<int, float>>& ranking)
	{
		_ranking.clear();//just in case it is not empty
		_ranking.reserve(ranking.size());
		_ranking.insert(_ranking.end(), ranking.begin(), ranking.end());
	}

	ranking_response::ranking_response()
	{}

	ranking_response::ranking_response(const std::string & uuid, const std::vector<std::pair<int, float>>& ranking)
		: _uuid(uuid), _ranking(ranking)
	{}

}

