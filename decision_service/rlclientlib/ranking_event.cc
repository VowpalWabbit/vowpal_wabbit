#include "ranking_event.h"
#include "ranking_response.h"
#include <ostream>

using namespace std;

namespace reinforcement_learning {
  void ranking_event::serialize(ostream& oss, const char* uuid, const char* context,
    ranking_response& resp, const std::string& model_id) {

    //add version and eventId
    oss << R"({"Version":"1","EventId":")" << uuid;

    //add action ids
    oss << R"(","a":[)";
    if ( resp.size() > 0 ) {
      for ( auto &r : resp )
        oss << r.action_id << ",";
      oss.seekp(-1, oss.cur);//remove last
    }

    //add probabilities
    oss << R"(],"c":)" << context << R"(,"p":[)";
    if ( resp.size() > 0 ) {
      for ( auto &r : resp )
        oss << r.probability << ",";
      oss.seekp(-1, oss.cur);//remove last
    }

    //add model id
    oss << R"(],"VWState":{"m":")" << model_id << R"("}})";
	}

  void outcome_event::serialize(ostream& oss, const char* uuid, const char* outcome_data)
	{
    oss << R"({"EventId":")" << uuid << R"(","v":")" << outcome_data << R"("})";
	}
}