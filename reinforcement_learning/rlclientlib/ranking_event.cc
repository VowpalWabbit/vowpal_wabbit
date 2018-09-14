#include "ranking_event.h"
#include "ranking_response.h"
#include "utility/data_buffer.h"

#include "explore_internal.h"
#include "hash.h"

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

  event::event(const char* event_id, float pass_prob)
    : _event_id(event_id)
    , _pass_prob(pass_prob)
  {}

  event::event(event&& other)
    : _event_id(std::move(other._event_id))
    , _pass_prob(other._pass_prob)
  {}

  event& event::operator=(event&& other) {
    if (&other != this) {
      _event_id = std::move(other._event_id);
      _pass_prob = other._pass_prob;
    }
    return *this;
  }

  event::~event() {}

  bool event::try_drop(float pass_prob, int drop_pass) {
    _pass_prob *= pass_prob;
    return prg(drop_pass) > pass_prob;
  }

  float event::prg(int drop_pass) const {
    const auto seed_str = _event_id + std::to_string(drop_pass);
    const auto seed = uniform_hash(seed_str.c_str(), seed_str.length(), 0);
    return exploration::uniform_random_merand48(seed);
  }

  ranking_event::ranking_event()
  { }

  ranking_event::ranking_event(u::data_buffer& oss, const char* event_id, const char* context,
    const ranking_response& response, float pass_prob)
    : event(event_id)
  {
    serialize(oss, event_id, context, response, _pass_prob);
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
    return prob_helper::patch(_body, 1 - _pass_prob);
  }

  void ranking_event::serialize(u::data_buffer& oss, const char* event_id, const char* context,
    const ranking_response& resp, float pass_prob) {

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
    oss << R"(],"VWState":{"m":")" << resp.get_model_id() << R"("},"pdrop":)" << prob_helper::format(1 - pass_prob) << R"(})";
	}

  outcome_event::outcome_event()
  { }

  outcome_event::outcome_event(utility::data_buffer& oss, const char* event_id, const char* outcome, float pass_prob)
    : event(event_id)
  {
    serialize(oss, event_id, outcome);
    _body = oss.str();
  }

  outcome_event::outcome_event(utility::data_buffer& oss, const char* event_id, float outcome, float pass_prob)
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

  void outcome_event::serialize(u::data_buffer& oss, const char* event_id, const char* outcome, float pass_prob) {
    oss << R"({"EventId":")" << event_id << R"(","v":)" << outcome << R"(})";
  }

  void outcome_event::serialize(u::data_buffer& oss, const char* event_id, float outcome, float pass_prob) {
    oss << R"({"EventId":")" << event_id << R"(","v":)" << outcome << R"(})";
  }
}
