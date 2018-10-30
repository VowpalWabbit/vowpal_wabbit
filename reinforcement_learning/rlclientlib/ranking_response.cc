#include "ranking_response.h"
#include "api_status.h"
#include "err_constants.h"

namespace reinforcement_learning {

  action_prob::action_prob(const size_t action_id, const float probability) : action_id { action_id }, probability { probability } {}

  ranking_response::ranking_response(char const* event_id)
    : _event_id { event_id }, _chosen_action_id { 0 } {}

  char const* ranking_response::get_event_id() const {
    return _event_id.c_str();
  }

  int ranking_response::get_chosen_action_id(size_t& action_id, api_status* status) const {
    if ( _ranking.empty() ) {
      RETURN_ERROR_LS(nullptr, status, action_not_found);
    }
    action_id = _chosen_action_id;
    return error_code::success;
  }

  int ranking_response::set_chosen_action_id(size_t action_id, api_status* status) {
    if ( action_id >= _ranking.size() ) {
      RETURN_ERROR_LS(nullptr, status, action_out_of_bounds) << " id:" << action_id << ", size:" << _ranking.size();
    }

    _chosen_action_id = action_id;
    return error_code::success;
  }

  void ranking_response::set_event_id(char const* event_id) {
    _event_id = event_id;
  }

  void ranking_response::push_back(const size_t action_id, const float prob) {
    _ranking.emplace_back(action_id, prob);
  }

  size_t ranking_response::size() const {
    return _ranking.size();
  }

  void ranking_response::set_model_id(const char* model_id) {
    _model_id = model_id;
  }

  const char* ranking_response::get_model_id() const {
    return _model_id.c_str();
  }

  void ranking_response::clear() {
    _event_id.clear();
    _chosen_action_id = 0;
    _model_id.clear();
    _ranking.clear();
  }

  ranking_response::ranking_response(ranking_response&& tmp) noexcept :
  _event_id(std::move(tmp._event_id)),
    _chosen_action_id(tmp._chosen_action_id),
    _model_id(std::move(tmp._model_id)),
    _ranking(std::move(tmp._ranking)) {}

  ranking_response& ranking_response::operator=(ranking_response&& tmp) noexcept {
    std::swap(_event_id, tmp._event_id);
    std::swap(_chosen_action_id, tmp._chosen_action_id);
    std::swap(_model_id, tmp._model_id);
    std::swap(_ranking, tmp._ranking);
    return *this;
  }

  using iterator = ranking_response::iterator;
  using const_iterator = ranking_response::const_iterator;

  iterator::iterator(ranking_response* p_resp)
    : _p_resp(p_resp), _idx(0) {}

  iterator::iterator(ranking_response* p_resp, const size_t idx)
    : _p_resp(p_resp), _idx(idx) {}

  iterator& iterator::operator++() {
    ++_idx;
    return *this;
  }

  bool iterator::operator!=(const iterator& other) const {
    return _idx != other._idx;
  }

  action_prob& iterator::operator*() {
    return _p_resp->_ranking[_idx];
  }

  bool iterator::operator<(const iterator& rhs) const {
    return _idx < rhs._idx;
  }

  int64_t iterator::operator-(const iterator& rhs) const {
    return _idx - rhs._idx;
  }

  iterator iterator::operator+(const uint32_t idx) const {
    return { _p_resp,_idx + idx };
  }

  const_iterator::const_iterator(const ranking_response* p_resp)
    : _p_resp { p_resp }, _idx { 0 } {}

  const_iterator::const_iterator(const ranking_response* p_resp, const size_t idx)
    : _p_resp { p_resp }, _idx { idx } {}

  const_iterator& ranking_response::const_iterator::operator++() {
    ++_idx;
    return *this;
  }

  bool const_iterator::operator!=(const const_iterator& other) const {
    return _idx != other._idx;
  }

  const action_prob& const_iterator::operator*() const {
    return _p_resp->_ranking[_idx];
  }

  bool const_iterator::operator<(const const_iterator& rhs) const {
    return _idx < rhs._idx;
  }

  int64_t const_iterator::operator-(const const_iterator& rhs) const {
    return _idx - rhs._idx;
  }

  const_iterator const_iterator::operator+(const uint32_t idx) const {
    return { _p_resp,_idx + idx };
  }

  const_iterator ranking_response::begin() const {
    return { this };
  }

  iterator ranking_response::begin() {
    return { this };
  }

  const_iterator ranking_response::end() const {
    return { this, _ranking.size() };
  }

  iterator ranking_response::end() {
    return { this, _ranking.size() };
  }
}
