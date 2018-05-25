#include "ranking_response_impl.h"

namespace reinforcement_learning {
 
  ranking_response_impl::ranking_response_impl(const std::string& uuid)
    : _uuid { uuid } {}

  bool ranking_response_impl::get_action(const size_t idx, int* action_id, float* prob) const {
    if ( idx < _ranking.size() ) {
      auto& t = _ranking[idx];
      *action_id = t.first;
      *prob = t.second;
      return true;
    }
    return false;
  }

  void ranking_response_impl::push_back(const int action_id, const float prob) {
    _ranking.push_back(std::pair<int,float>(action_id, prob));
  }

  size_t ranking_response_impl::size() const { return _ranking.size(); }

  bool ranking_response_impl::get_top_action_id(int* action_id) const {
    if ( !_ranking.empty() ) {
      auto& t = _ranking[0];
      *action_id = t.first;
      return true;
    }
    return false;
  }
}
