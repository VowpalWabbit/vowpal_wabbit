#include "action_flags.h"
#include "ranking_event.h"
#include "utility/data_buffer.h"

#include "explore_internal.h"
#include "hash.h"

#include <sstream>
#include <iomanip>

using namespace std;

namespace reinforcement_learning {
  namespace u = utility;

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

  ranking_event::ranking_event(const char* event_id, bool deferred_action, float pass_prob, const char* context, const ranking_response& response)
    : event(event_id, pass_prob)
    , _deferred_action(deferred_action)
    , _model_id(response.get_model_id())
  {
    for (auto const &r : response) {
      _action_ids_vector.push_back(r.action_id + 1);
      _probilities_vector.push_back(r.probability);
    }

  string context_str(context);
    copy(context_str.begin(), context_str.end(), std::back_inserter(_context));
  }

  ranking_event::ranking_event(ranking_event&& other)
    : event(std::move(other))
    , _deferred_action(other._deferred_action)
    , _context(std::move(other._context))
    , _action_ids_vector(std::move(other._action_ids_vector))
    , _probilities_vector(std::move(other._probilities_vector))
    , _model_id(std::move(other._model_id))
  { }

  ranking_event& ranking_event::operator=(ranking_event&& other) {
    if (&other != this) {
      event::operator=(std::move(other));
      _deferred_action = std::move(other._deferred_action);
      _context = std::move(other._context);
      _action_ids_vector = std::move(other._action_ids_vector);
      _probilities_vector = std::move(other._probilities_vector);
      _model_id = std::move(other._model_id);
    }
    return *this;
  }

  std::vector<unsigned char> ranking_event::get_context() {
    return _context;
  }

  std::vector<uint64_t> ranking_event::get_action_ids() {
    return _action_ids_vector;
  }

  std::vector<float> ranking_event::get_probabilities() {
    return _probilities_vector;
  }

  std::string ranking_event::get_model_id() {
    return _model_id;
  }

  bool ranking_event::get_defered_action() {
    return _deferred_action;
  }

  ranking_event ranking_event::choose_rank(const char* event_id, const char* context,
    unsigned int flags, const ranking_response& resp, float pass_prob) {
    return ranking_event(event_id, flags & action_flags::DEFERRED, pass_prob, context, resp);
  }

  std::string ranking_event::str() {
    u::data_buffer oss;

    oss << R"({"Version":"1","EventId":")" << _event_id << R"(")";

    if (_deferred_action) {
      oss << R"(,"DeferredAction":true)";
    }

    //add action ids
    oss << R"(,"a":[)";
    for (auto id : _action_ids_vector) {
      oss << id + 1 << ",";
    }
    //remove trailing ,
    oss.remove_last();

    //add probabilities
    oss << R"(],"c":)" << std::string(_context.begin(), _context.end()) << R"(,"p":[)";
    for (auto prob : _probilities_vector) {
      oss << prob << ",";
    }
    //remove trailing ,
    oss.remove_last();

    //add model id
    oss << R"(],"VWState":{"m":")" << _model_id << R"("})";

    return oss.str();
  }

  outcome_event::outcome_event()
  { }

  outcome_event::outcome_event(const char* event_id, float pass_prob, const char* outcome, bool deferred_action)
    : event(event_id, pass_prob)
    , _outcome(outcome)
    , _deferred_action(deferred_action)
  {
  }

  outcome_event::outcome_event(const char* event_id, float pass_prob, float outcome, bool deferred_action)
    : event(event_id, pass_prob)
    , _deferred_action(deferred_action)
  {
    _outcome = std::to_string(outcome);
  }

  outcome_event::outcome_event(outcome_event&& other)
    : event(std::move(other))
    , _outcome(std::move(other._outcome))
  { }

  outcome_event& outcome_event::operator=(outcome_event&& other) {
    if (&other != this) {
      event::operator=(std::move(other));
      _outcome = std::move(other._outcome);
    }
    return *this;
  }

  std::string outcome_event::str() {
    u::data_buffer oss;
    std::string deferred_action = _deferred_action ? "true" : false;
    oss << R"({"EventId":")" << _event_id << R"(","v":)" << _outcome << R"(","DeferredAction":)" << deferred_action << R"(})";
    return oss.str();
  }

  outcome_event outcome_event::report_outcome(const char* event_id, const char* outcome, float pass_prob) {
    return outcome_event(event_id, pass_prob, outcome, true);
  }

  outcome_event outcome_event::report_outcome(const char* event_id, float outcome, float pass_prob) {
    return outcome_event(event_id, pass_prob, outcome, true);
  }

  outcome_event outcome_event::report_action_taken(const char* event_id, float pass_prob) {
    return outcome_event(event_id, pass_prob, "", false);
  }

  std::string outcome_event::get_outcome() {
    return _outcome;
  }

  bool outcome_event::get_deferred_action() {
    return _deferred_action;
  }
}
