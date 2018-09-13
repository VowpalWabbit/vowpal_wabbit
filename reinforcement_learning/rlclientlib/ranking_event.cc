#include "ranking_event.h"
#include "ranking_response.h"
#include "utility/data_buffer.h"
#include <sstream>
#include <iomanip>

using namespace std;

namespace reinforcement_learning {
  namespace u = utility;

  class prob_helper {
  public:
    static const int precision = 4;

    static std::string format(float value) {
      stringstream ss;
      ss << std::fixed << std::setprecision(precision) << value;
      return ss.str();
    }

    static size_t length() {
      return precision + 2;
    }
  };

  string pdrop_patcher::patch(string& message, float pdrop) {
    return message.replace(message.size() - prob_helper::length() - 1, prob_helper::length(), prob_helper::format(pdrop));
  }

  void ranking_event::serialize(u::data_buffer& oss, const char* event_id, const char* context,
    const ranking_response& resp, float pdrop) {

    //add version and eventId
    oss << R"({"Version":"1","EventId":")" << event_id;

    //add action ids
    oss << R"(","a":[)";
    if ( resp.size() > 0 ) {
      for ( auto const &r : resp )
        oss << r.action_id + 1 << ",";
      oss.remove_last();//remove trailing ,
    }

    //add probabilities
    oss << R"(],"c":)" << context << R"(,"p":[)";
    if ( resp.size() > 0 ) {
      for ( auto const &r : resp )
        oss << r.probability << ",";
      oss.remove_last();//remove trailing ,
    }

    //add model id
    oss << R"(],"VWState":{"m":")" << resp.get_model_id() << R"("},"pdrop":)" << prob_helper::format(pdrop) << R"(})";
	}

  void outcome_event::serialize(u::data_buffer& oss, const char* event_id, const char* outcome, float pdrop) {
    oss << R"({"EventId":")" << event_id << R"(","v":)" << outcome << R"(,"pdrop":)" << prob_helper::format(pdrop) << R"(})";
  }

  void outcome_event::serialize(u::data_buffer& oss, const char* event_id, float outcome, float pdrop) {
    oss << R"({"EventId":")" << event_id << R"(","v":)" << outcome << R"(,"pdrop":)" << prob_helper::format(pdrop) << R"(})";
  }
}
