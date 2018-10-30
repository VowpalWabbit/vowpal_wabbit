#pragma once
#include <string>
#include "ranking_response.h"
#include "generated/RankingEvent_generated.h"
#include "generated/OutcomeEvent_generated.h"

using VW::Events::RankingEvent;
using VW::Events::OutcomeEvent;

namespace reinforcement_learning {
  namespace utility { class data_buffer; }

  class event {
  public:
    event();
    event(const char* event_id, float pass_prob = 1);
    event(event&& other);
    std::string get_event_id() {
      return _event_id;
    }

    event& operator=(event&& other);
    virtual ~event();

    virtual bool try_drop(float pass_prob, int drop_pass);

  protected:
    float prg(int drop_pass) const;

  protected:
    std::string _event_id;
    float _pass_prob;
  };

  class ranking_response;

  //serializable ranking event
  class ranking_event : public event {
  public:
    ranking_event();
    ranking_event(ranking_event&& other);
    ranking_event& operator=(ranking_event&& other);

    flatbuffers::Offset<RankingEvent> serialize_eventhub_message(flatbuffers::FlatBufferBuilder& builder);
    std::string str();

  public:
    static ranking_event choose_rank(const char* event_id, const char* context,
      unsigned int flags, const ranking_response& resp, float pass_prob = 1);

  private:
    ranking_event(const char* event_id, bool deferred_action, float pass_prob, const char* context, const ranking_response& response);

    std::string _context;
    std::vector<uint64_t> _a_vector;
    std::vector<float> _p_vector;
    std::string _model_id;
    bool _deferred_action;
  };

  //serializable outcome event
  class outcome_event : public event {
  public:
    outcome_event();

    outcome_event(outcome_event&& other);
    outcome_event& operator=(outcome_event&& other);

    std::string str();
    flatbuffers::Offset<OutcomeEvent> serialize_eventhub_message(flatbuffers::FlatBufferBuilder& builder);

  public:
    static outcome_event report_action_taken(const char* event_id, float pass_prob = 1);

    static outcome_event report_outcome(const char* event_id, const char* outcome, float pass_prob = 1);
    static outcome_event report_outcome(const char* event_id, float outcome, float pass_prob = 1);

  private:
    outcome_event(const char* event_id, float pass_prob, const char* outcome);
    outcome_event(const char* event_id, float pass_prob, float outcome);

  private:
    std::string _outcome;
  };
}
