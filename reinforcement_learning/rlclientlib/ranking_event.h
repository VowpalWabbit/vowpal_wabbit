#pragma once

#include <string>
#include <vector>

namespace reinforcement_learning {
  class ranking_response;

  //serializable ranking event
	class ranking_event {
	public:
		static void serialize(std::ostream& oss, const char* uuid, const char* context,
      ranking_response& resp);
	};

	//serializable outcome event
	class outcome_event {
	public:
    static void serialize(std::ostream& oss, const char* uuid, const char* outcome_data);
    static void serialize(std::ostream& oss, const char* uuid, float reward);
  };
}
