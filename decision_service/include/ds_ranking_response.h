#pragma once

#include <string>
#include <vector>

namespace decision_service {

	//Ranking response returned by the decision service
	class ranking_response {

	public:
		ranking_response(const std::string& uuid, const std::vector<std::pair<int, float>>& ranking);

		const std::string& uuid() const;                           //unique id
		int top_action_id() const;                                 //id of the top action chosen by the ds
		const std::vector<std::pair<int, float>>& ranking() const; //ranked pairs of action_id/probability

	private:
		std::string _uuid;
		int _top_action_id;
		std::vector<std::pair<int, float>> _ranking;
	};
}
