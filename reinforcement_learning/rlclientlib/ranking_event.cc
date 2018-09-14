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
    
    static string patch(string& message, float pdrop) {
      return message.replace(message.size() - prob_helper::length() - 1, prob_helper::length(), prob_helper::format(pdrop));
    }
  };

  event::event()
  {}

  event::event(const char* event_id, float pdrop) 
    : _event_id(event_id)
    , _pdrop(pdrop)
  {}

  event::event(event&& other) 
    : _event_id(std::move(other._event_id))
    , _pdrop(other._pdrop)
  {}

  event& event::operator=(event&& other) {
    if (&other != this) {
      _event_id = std::move(other._event_id);
      _pdrop = other._pdrop;
    }
    return *this;
  }

  event::~event() {}

  bool event::try_drop(float drop_prob, int drop_pass) {
    return false;
  }

  ranking_event::ranking_event()
  { }

  ranking_event::ranking_event(u::data_buffer& oss, const char* event_id, const char* context,
    const ranking_response& response, float pdrop)
    : event(event_id)
  {
    serialize(oss, event_id, context, response, _pdrop);
    _body = oss.str();
  }

  ranking_event::ranking_event(ranking_event&& other)
    : event(std::move(other))
    , _body(std::move(other._body))
  {}

  ranking_event& ranking_event::operator=(ranking_event&& other) {
    if (&other != this) {
      event::operator=(std::move(other));
      _body = std::move(other._body);
    }
    return *this;
  }

  std::string ranking_event::str() {
    return prob_helper::patch(_body, _pdrop);
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

  outcome_event::outcome_event()
  { }

  outcome_event::outcome_event(utility::data_buffer& oss, const char* event_id, const char* outcome, float pdrop) 
    : event(event_id)
  {
    serialize(oss, event_id, outcome);
    _body = oss.str();
  }

  outcome_event::outcome_event(utility::data_buffer& oss, const char* event_id, float outcome, float pdrop)
    : event(event_id)
  {
    serialize(oss, event_id, outcome);
    _body = oss.str();
  }

  outcome_event::outcome_event(outcome_event&& other) 
    : event(std::move(other))
    , _body(std::move(other._body)) 
  { }

  outcome_event& outcome_event::operator=(outcome_event&& other) {
    if (&other != this) {
      event::operator=(std::move(other));
      _body = std::move(other._body);
    }
    return *this;
  }

  std::string outcome_event::str() {
    return _body;
  }

  void outcome_event::serialize(u::data_buffer& oss, const char* event_id, const char* outcome, float pdrop) {
    oss << R"({"EventId":")" << event_id << R"(","v":)" << outcome << R"(,"pdrop":)" << prob_helper::format(pdrop) << R"(})";
  }

  void outcome_event::serialize(u::data_buffer& oss, const char* event_id, float outcome, float pdrop) {
    oss << R"({"EventId":")" << event_id << R"(","v":)" << outcome << R"(,"pdrop":)" << prob_helper::format(pdrop) << R"(})";
  }
}
