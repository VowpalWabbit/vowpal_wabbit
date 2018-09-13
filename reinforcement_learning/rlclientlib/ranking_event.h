#pragma once
#include <string>

namespace reinforcement_learning {
  namespace utility { class data_buffer; }

  class ranking_response;

  //serializable ranking event
  class ranking_event {
  public:
    static void serialize(utility::data_buffer& oss, const char* event_id, const char* context,
      const ranking_response& resp, float pdrop = 1);
  };

  //serializable outcome event
  class outcome_event {
  public:
    static void serialize(utility::data_buffer& oss, const char* event_id, const char* outcome, float pdrop = 1);
    static void serialize(utility::data_buffer& oss, const char* event_id, float outcome, float pdrop = 1);
  };

  class pdrop_patcher {
  public:
    static std::string patch(std::string& msg, float pdrop);
  };
}
