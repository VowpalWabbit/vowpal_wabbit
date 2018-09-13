#pragma once
#include <string>

namespace reinforcement_learning {
  class message {
  private:
    std::string _event_id;
    std::string _body;
    float _survival_prob;

  public:
    message();
    message(const char* event_id, std::string&& body);
    message(message&& other);

    message& operator= (message&& other);

    bool try_drop(float drop_prob, int drop_pass);

    std::string str();
  };
}
