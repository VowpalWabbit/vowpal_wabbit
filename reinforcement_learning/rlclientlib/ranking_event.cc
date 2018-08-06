#include "ranking_event.h"
#include "ranking_response.h"
#include "utility/data_buffer.h"

using namespace std;

namespace reinforcement_learning {
  namespace u = utility;

  void ranking_event::serialize(u::data_buffer& oss, const char* uuid, const char* context,
    ranking_response& resp) {

    //add version and eventId
    oss << R"({"Version":"1","EventId":")" << uuid;

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
    oss << R"(],"VWState":{"m":")" << resp.get_model_id() << R"("}})";
	}

  void outcome_event::serialize(u::data_buffer& oss, const char* uuid, const char* outcome_data) {
    oss << R"({"EventId":")" << uuid << R"(","v":")" << outcome_data << R"("})";
  }

  void outcome_event::serialize(u::data_buffer& oss, const char* uuid, float reward) {
    oss << R"({"EventId":")" << uuid << R"(","v":)" << reward << R"(})";
  }
}