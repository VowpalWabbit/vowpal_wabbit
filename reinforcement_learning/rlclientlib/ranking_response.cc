#include "ranking_response.h"
#include "ranking_response_impl.h"
#include "api_status.h"
#include "err_constants.h"

namespace reinforcement_learning {

  ranking_response::ranking_response()
    : _pimpl {new ranking_response_impl()} {}

  ranking_response::ranking_response(char const* event_id)
    : _pimpl{new ranking_response_impl(event_id)} {}

  ranking_response::~ranking_response() {
    delete _pimpl;
  }

  char const* ranking_response::get_event_id() const {
    return _pimpl->_event_id.c_str();
  }

  int ranking_response::get_chosen_action_id(size_t& action_id, api_status* status) const {
    if (!_pimpl->get_chosen_action_id(action_id)) {
      RETURN_ERROR_LS(status, action_not_found);
    }
    return error_code::success;
  }

  int ranking_response::set_chosen_action_id(size_t action_id, api_status* status) {
    return _pimpl->set_chosen_action_id(action_id, status);
  }

  void ranking_response::set_event_id(char const* event_id) {
    _pimpl->_event_id = event_id;
  }

  void ranking_response::push_back(const size_t action_id, const float prob) {
    _pimpl->push_back(action_id, prob);
  }

  size_t ranking_response::size() const {
    return _pimpl->size();
  }

  void ranking_response::set_model_id(const char* model_id) {
    _pimpl->set_model_id(model_id);
  }

  const char* ranking_response::get_model_id() const {
    return _pimpl->get_model_id();
  }

  void ranking_response::clear() {
    _pimpl->reset();
  }

  ranking_response::ranking_response(ranking_response&& tmp) noexcept {
    _pimpl = tmp._pimpl;
    tmp._pimpl = nullptr;
  }

  ranking_response& ranking_response::operator=(ranking_response&& tmp) noexcept {
    const auto swap = _pimpl;
    _pimpl = tmp._pimpl;
    tmp._pimpl = swap;
    return *this;
  }

  ranking_response::ranking_iterator::ranking_iterator(ranking_response_impl* p_resp_impl)
    : _p_resp_impl(p_resp_impl), _idx(0) { }

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
    size_t action_id;
    float prob;
    _p_resp_impl->get_action(_idx, &action_id, &prob);
    return action_prob{action_id, prob};
  }

  ranking_response::ranking_iterator ranking_response::begin() const {
    return {_pimpl};
  }

  ranking_response::ranking_iterator ranking_response::end() const {
    return {_pimpl, _pimpl->size()};
  }
}
