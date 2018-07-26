#include "ranking_event.h"
#include "ranking_response.h"
#include <ostream>

using namespace std;

namespace reinforcement_learning {
  void ranking_event::serialize(ostream& oss, const char* uuid, const char* context,
    ranking_response& resp) {

    //add version and eventId
    oss << R"({"Version":"1","EventId":")" << uuid;

    //add action ids
    oss << R"(","a":[)";
    if ( resp.size() > 0 ) {
      for ( auto const &r : resp )
        oss << r.action_id + 1 << ",";
      oss.seekp(-1, oss.cur);//remove last
    }

    //add probabilities
    oss << R"(],"c":)" << context << R"(,"p":[)";
    if ( resp.size() > 0 ) {
      for ( auto const &r : resp )
        oss << r.probability << ",";
      oss.seekp(-1, oss.cur);//remove last
    }

    //add model id
    oss << R"(],"VWState":{"m":")" << resp.get_model_id() << R"("}})" << std::ends;
	}

  void outcome_event::serialize(ostream& oss, const char* uuid, const char* outcome_data) {
    oss << R"({"EventId":")" << uuid << R"(","v":")" << outcome_data << R"("})" << std::ends;
  }

  void outcome_event::serialize(ostream& oss, const char* uuid, float reward) {
    oss << R"({"EventId":")" << uuid << R"(","v":)" << reward << R"(})" << std::ends;
  }
}