#include "ranking_response_impl.h"
#include <object_factory.h>

namespace reinforcement_learning {
 
  ranking_response_impl::ranking_response_impl(const std::string& uuid)
  : _uuid {uuid}, _chosen_action_id{0} {}

  bool ranking_response_impl::get_action(const size_t idx, size_t* action_id, float* prob) const {
    if ( idx < _ranking.size() ) {
      auto& t = _ranking[idx];
      *action_id = t.first;
      *prob = t.second;
      return true;
    }
    return false;
  }

  void ranking_response_impl::push_back(const size_t action_id, const float prob) {
    _ranking.emplace_back(action_id, prob);
  }

  size_t ranking_response_impl::size() const { return _ranking.size(); }
  
  void ranking_response_impl::set_model_id(const char* model_id) {
    _model_id = model_id;
  }

  const char* ranking_response_impl::get_model_id() const {
    return _model_id.c_str();
  }

  void ranking_response_impl::reset() {
    _uuid.clear();
    _chosen_action_id = 0;
    _model_id.clear();
    _ranking.clear();
  }

  bool ranking_response_impl::get_choosen_action_id(size_t& action_id) const {
    if ( !_ranking.empty() ) {
      action_id =_chosen_action_id;
      return true;
    }
    return false;
  }

  int ranking_response_impl::set_choosen_action_id(size_t action_id, api_status* status) {
    if ( action_id >= _ranking.size() ) {
      RETURN_ERROR_LS(status, action_out_of_bounds) << " id:" << action_id << ", size:" << _ranking.size();
    }

    _chosen_action_id = action_id;
    return error_code::success;
  }
}
