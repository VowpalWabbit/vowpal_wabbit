#pragma once
#include <string>
#include <vector>
#include "live_model.h"

class action_ranking;

namespace reinforcement_learning {
  class ranking_response_impl {
  public:
    ranking_response_impl() = default;
    explicit ranking_response_impl(const std::string& event_id);

    bool get_chosen_action_id(size_t& action_id) const;
    int set_chosen_action_id(size_t action_id, api_status* status);
    bool get_action(const size_t idx, size_t* action_id, float* prob) const;
    void push_back(const size_t action_id, const float prob);
    size_t size() const;
    void set_model_id(const char* model_id);
    const char* get_model_id() const;
    void reset();

    ranking_response_impl(const ranking_response_impl&) = delete;
    ranking_response_impl(ranking_response_impl&&) = delete;
    ranking_response_impl& operator=(const ranking_response_impl&) = delete;
    ranking_response_impl& operator=(ranking_response_impl&&) = delete;

  private:
    std::string _event_id;
    size_t _chosen_action_id;
    std::string _model_id;
    using coll_type = std::vector<std::pair<size_t, float>>;
    coll_type _ranking;
    friend class ranking_response;
  };
}
