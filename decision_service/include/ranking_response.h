#pragma once

#include <string>
#include <vector>

namespace reinforcement_learning {

	//Ranking response returned by the decision service
	class ranking_response {

	public:
		ranking_response();
		ranking_response(const std::string& uuid, const std::vector<std::pair<int, float>>& ranking);

		const std::string& get_uuid() const;                           //unique id
		int get_top_action_id() const;                                 //id of the top action chosen by the ds
		const std::vector<std::pair<int, float>>& get_ranking() const; //ranked pairs of action_id/probability

		void set_uuid(const std::string& uuid);
		void set_ranking(const std::vector<std::pair<int, float>>& ranking);

	private:
		std::string _uuid;
		int _top_action_id;
		std::vector<std::pair<int, float>> _ranking;
	};
}
