#pragma once

namespace reinforcement_learning {
  namespace utility { class data_buffer; }

  class ranking_response;

  //serializable ranking event
	class ranking_event {
	public:
		static void serialize(utility::data_buffer& oss, const char* event_id, const char* context,
      const ranking_response& resp);
	};

	//serializable outcome event
	class outcome_event {
	public:
    static void serialize(utility::data_buffer& oss, const char* event_id, const char* outcome);
    static void serialize(utility::data_buffer& oss, const char* event_id, float outcome);
  };
}
