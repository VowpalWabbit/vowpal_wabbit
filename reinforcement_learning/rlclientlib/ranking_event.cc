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

  ranking_event::ranking_event(const char* event_id, float pass_prob, const std::string& body)
    : event(event_id, pass_prob)
    , _body(body)
  { }

  ranking_event::ranking_event(u::data_buffer& oss, const char* event_id, const char* context,
    const ranking_response& response, float pass_prob)
    : event(event_id, pass_prob)
  {
    _context = std::string(context);
    for (auto const &r : response) {
      _a_vector.push_back(r.action_id);
      _p_vector.push_back(r.probability);
    }
    _model_id = std::string(response.get_model_id());
  }

  ranking_event::ranking_event(ranking_event&& other)
    : event(std::move(other))
    , _body(std::move(other._body))
    , _context(std::move(other._context))
    , _a_vector(std::move(other._a_vector))
    , _p_vector(std::move(other._p_vector))
    , _model_id(std::move(other._model_id))
  { }

  ranking_event& ranking_event::operator=(ranking_event&& other) {
    if (&other != this) {
      event::operator=(std::move(other));
      _body = std::move(other._body);
      _context = std::move(other._context);
      _a_vector = std::move(other._a_vector);
      _p_vector = std::move(other._p_vector);
      _model_id = std::move(other._model_id);
    }
    return *this;
  }

  flatbuffers::Offset<RankingEvent> ranking_event::serialize_eventhub_message(flatbuffers::FlatBufferBuilder& builder) {
    short version = 1;
    auto event_id_offset = builder.CreateString(_event_id);

    auto a_vector_offset = builder.CreateVector(_a_vector);
    auto p_vector_offset = builder.CreateVector(_p_vector);

    auto context_offset = builder.CreateString(_context);

    auto vw_state_offset = VW::Events::CreateVWStateType(builder, builder.CreateString(_model_id));

    return VW::Events::CreateRankingEvent(builder, version, event_id_offset, a_vector_offset, context_offset, p_vector_offset, vw_state_offset);
  }

  ranking_event ranking_event::choose_rank(u::data_buffer& oss, const char* event_id, const char* context,
    unsigned int flags, const ranking_response& resp, float pass_prob) {

    //add version and eventId
    oss << R"({"Version":"1","EventId":")" << event_id << R"(")";

    if (flags & action_flags::DEFERRED) {
      oss << R"(,"DeferredAction":true)";
    }

    //add action ids
    oss << R"(,"a":[)";
    if (resp.size() > 0) {
      for (auto const &r : resp)
        oss << r.action_id + 1 << ",";
      oss.remove_last();//remove trailing ,
    }

    //add probabilities
    oss << R"(],"c":)" << context << R"(,"p":[)";
    if (resp.size() > 0) {
      for (auto const &r : resp)
        oss << r.probability << ",";
      oss.remove_last();//remove trailing ,
    }

    //add model id
    oss << R"(],"VWState":{"m":")" << resp.get_model_id() << R"("})";
    return ranking_event(event_id, pass_prob, oss.str());
  }

  void ranking_event::serialize(u::data_buffer& oss) {
    oss << _body;
    if (_pass_prob < 1) {
      oss << R"(,"pdrop":)" << (1 - _pass_prob);
    }
    oss << R"(})";
  }

  outcome_event::outcome_event()
  { }

  outcome_event::outcome_event(const char* event_id, float pass_prob, const std::string& body)
    : event(event_id, pass_prob)
    , _body(body)
  { }

  outcome_event::outcome_event(utility::data_buffer& oss, const char* event_id, const char* outcome, float pass_prob)
    : event(event_id, pass_prob)
  {
    _outcome = std::string(outcome);
  }

  outcome_event::outcome_event(utility::data_buffer& oss, const char* event_id, float outcome, float pass_prob)
    : event(event_id)
  {
    _outcome = std::to_string(outcome);
  }

  outcome_event::outcome_event(outcome_event&& other)
    : event(std::move(other))
    , _body(std::move(other._body))
    , _outcome(std::move(other._outcome))
  { }

  outcome_event& outcome_event::operator=(outcome_event&& other) {
    if (&other != this) {
      event::operator=(std::move(other));
      _body = std::move(other._body);
      _outcome = std::move(other._outcome);
    }
    return *this;
  }

  flatbuffers::Offset<OutcomeEvent> outcome_event::serialize_eventhub_message(flatbuffers::FlatBufferBuilder& builder) {
    auto event_id_offset = builder.CreateString(_event_id);
    auto outcome_offset = builder.CreateString(_outcome);
    return VW::Events::CreateOutcomeEvent(builder, event_id_offset, outcome_offset);
  }

  void outcome_event::serialize(u::data_buffer& oss) {
    oss << _body;
  }

  outcome_event outcome_event::report_outcome(u::data_buffer& oss, const char* event_id, const char* outcome, float pass_prob) {
    oss << R"({"EventId":")" << event_id << R"(","v":)" << outcome << R"(})";
    return outcome_event(event_id, pass_prob, oss.str());
  }

  outcome_event outcome_event::report_outcome(u::data_buffer& oss, const char* event_id, float outcome, float pass_prob) {
    oss << R"({"EventId":")" << event_id << R"(","v":)" << outcome << R"(})";
    return outcome_event(event_id, pass_prob, oss.str());
  }

  outcome_event outcome_event::report_action_taken(utility::data_buffer& oss, const char* event_id, float pass_prob) {
    oss << R"({"EventId":")" << event_id << R"(","DeferredAction":false})";
    return outcome_event(event_id, pass_prob, oss.str());
  }
}
