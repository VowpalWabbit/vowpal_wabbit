#include "message.h"
#include "ranking_event.h"

namespace reinforcement_learning {
  message::message() {}

  message::message(const char* event_id, std::string&& body) 
    : _event_id(event_id)
    , _body(body)
    , _survival_prob(1.0)
  { }

  bool message::try_drop(float drop_prob, int drop_pass) {
    _survival_prob *= (1 - drop_prob);
    return false;
  }

  message::message(message&& other)
    : _event_id(std::move(other._event_id))
    , _body(std::move(other._body))
    , _survival_prob(other._survival_prob)
  {
  }

  message& message::operator= (message&& other) {
    _event_id = std::move(other._event_id);
    _body = std::move(other._body);
    _survival_prob = other._survival_prob;
    return *this;
  }

  std::string message::str() {
    return pdrop_patcher::patch(_body, 1 - _survival_prob);
  }
}