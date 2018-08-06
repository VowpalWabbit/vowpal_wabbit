#pragma once

namespace reinforcement_learning {
  namespace utility { class data_buffer; }

  class ranking_response;

  //serializable ranking event
	class ranking_event {
	public:
		static void serialize(utility::data_buffer& oss, const char* uuid, const char* context,
      ranking_response& resp);
	};

	//serializable outcome event
	class outcome_event {
	public:
    static void serialize(utility::data_buffer& oss, const char* uuid, const char* outcome_data);
    static void serialize(utility::data_buffer& oss, const char* uuid, float reward);
  };
}
