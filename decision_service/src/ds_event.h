#pragma once

#include <string>
#include <vector>

namespace decision_service {

	class ranking_event {
	public:
		ranking_event(const char* uuid, const char* context, const std::vector<std::pair<int, float>>& ranking, const std::string& model_id);
		std::string serialize();

	private:
		const char* _uuid;
		const char* _context;
		const std::vector<std::pair<int, float>>& _ranking;
		const std::string& _model_id;
	};

	class outcome_event {
	public:
		outcome_event(const char* uuid, const char* outcome_data);
		std::string serialize();

	private:
		const char* _uuid;
		const char* _outcome_data;
	};

}
