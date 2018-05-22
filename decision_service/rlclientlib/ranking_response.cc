#include "ranking_response.h"
#include "ranking_response_impl.h"
#include "api_status.h"

namespace reinforcement_learning {

  ranking_response::ranking_response()
    : _pimpl { new ranking_response_impl() } {}

  ranking_response::ranking_response(char const* uuid) 
  : _pimpl{ new ranking_response_impl(uuid) } {}

  char const * ranking_response::get_uuid() const {
    return _pimpl->_uuid.c_str();
	}

	int ranking_response::get_top_action_id(api_status* status) const {
    int action_id;
    if ( _pimpl->get_top_action_id(&action_id) )
      return action_id;

    api_status::try_update(status, error_code::action_not_found, 
      "No actions found in action collection");
    
    return -1;
	}

	void ranking_response::set_uuid(char const * uuid) {
    _pimpl->_uuid = uuid;
	}

  void ranking_response::push_back(const int action_id, const float prob) {
    _pimpl->push_back(action_id, prob);
  }

  int ranking_response::size() const { return _pimpl->size(); }

  ranking_response::ranking_iterator::ranking_iterator(ranking_response_impl* p_resp_impl) 
    :_p_resp_impl(p_resp_impl), _idx(0) { }

  ranking_response::ranking_iterator::ranking_iterator(ranking_response_impl* p_resp_impl, size_t idx)
    : _p_resp_impl(p_resp_impl), _idx(idx) { }

  ranking_response::ranking_iterator& ranking_response::ranking_iterator::operator++() {
    ++_idx;
    return *this;
  }
  
  bool ranking_response::ranking_iterator::operator!=(const ranking_iterator& other) const {
    return _idx != other._idx;
  }

  action_prob ranking_response::ranking_iterator::operator*() const {
    int action_id;
    float prob;
    this->_p_resp_impl->get_action(_idx, &action_id, &prob);
    return action_prob { action_id, prob };
  }

  ranking_response::ranking_iterator ranking_response::begin() const {
    return ranking_iterator(_pimpl);
  }

  ranking_response::ranking_iterator ranking_response::end() const {
    return ranking_iterator(_pimpl, _pimpl->size());
  }
}

