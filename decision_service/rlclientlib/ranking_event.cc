#include "ranking_event.h"

#include <sstream>


namespace reinforcement_learning {
	ranking_event::ranking_event(const char* uuid, const char* context, const std::vector<std::pair<int, float>>& ranking, const std::string& model_id)
		: _uuid(uuid), _context(context), _ranking(ranking), _model_id(model_id)
	{}

	std::string ranking_event::serialize()
	{
		std::ostringstream oss;

		//add version and eventId
		oss << R"({"Version":"1","EventId":")" << _uuid;

		//add action ids
		oss << R"(","a":[)";
		if (_ranking.size() > 0)
		{
			for (auto &r : _ranking)
				oss << r.first << ",";
			oss.seekp(-1, oss.cur);//remove last
		}

		//add probabilities
		oss << R"(],"c":)" << _context << R"(,"p":[)";
		if (_ranking.size() > 0)
		{
			for (auto &r : _ranking)
				oss << r.second << ",";
			oss.seekp(-1, oss.cur);//remove last
		}

		//add model id
		oss << R"(],"VWState":{"m":")" << _model_id << R"("}})";

		return oss.str();
	}

	outcome_event::outcome_event(const char* uuid, const char* outcome_data)
		:_uuid(uuid), _outcome_data(outcome_data)
	{}

	std::string outcome_event::serialize()
	{
		std::ostringstream oss;
		oss << R"({"EventId":")" << _uuid << R"(","v":")" << _outcome_data << R"("})";
		return oss.str();
	}
}