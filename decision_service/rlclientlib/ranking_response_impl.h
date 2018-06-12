#pragma once
#include <string>
#include <vector>
#include "live_model.h"

class action_ranking;

namespace reinforcement_learning {
  class ranking_response_impl {
  public:
    ranking_response_impl() = default;
    ranking_response_impl(const std::string & uuid);
    ~ranking_response_impl();

    bool get_top_action_id(int* action_id) const;
    bool get_action(const size_t idx, int* action_id, float* prob) const;
    void push_back(const int action_id, const float prob);
    size_t size() const;

    ranking_response_impl(const ranking_response_impl &) = delete;
    ranking_response_impl(ranking_response_impl &&) = delete;
    ranking_response_impl& operator=(const ranking_response_impl &) = delete;
    ranking_response_impl& operator=(ranking_response_impl &&) = delete;

    private:
    std::string _uuid;
    using coll_type = std::vector<std::pair<int, float>>;
    coll_type _ranking;
    friend class ranking_response;
  };
}
