#define BOOST_TEST_DYN_LINK

#include "mock_util.h"

#include "constants.h"
#include "err_constants.h"
#include "ranking_response.h"
#include "model_mgmt.h"

namespace r = reinforcement_learning;
namespace m = r::model_management;
namespace u = r::utility;

using namespace fakeit;

std::unique_ptr<fakeit::Mock<r::i_sender>> get_mock_sender() {
  auto mock = std::unique_ptr<fakeit::Mock<r::i_sender>>(
    new fakeit::Mock<r::i_sender>());

  When(Method((*mock), init)).AlwaysReturn(r::error_code::success);
  When(Method((*mock), send)).AlwaysReturn(r::error_code::success);
  Fake(Dtor((*mock)));

  return mock;
}

std::unique_ptr<fakeit::Mock<r::i_sender>> get_mock_sender(std::vector<std::vector<unsigned char>>& recorded_messages) {
  auto mock = std::unique_ptr<fakeit::Mock<r::i_sender>>(
    new fakeit::Mock<r::i_sender>());

  When(Method((*mock), init)).AlwaysReturn(r::error_code::success);
  When(Method((*mock), send)).AlwaysDo(
    [&recorded_messages](std::vector<unsigned char> &message, reinforcement_learning::api_status* status) {
    recorded_messages.push_back(message);
    return r::error_code::success;
  });
  Fake(Dtor((*mock)));

  return mock;
}


std::unique_ptr<fakeit::Mock<m::i_data_transport>> get_mock_data_transport() {
  auto mock = std::unique_ptr<fakeit::Mock<m::i_data_transport>>(
    new fakeit::Mock<m::i_data_transport>());

  When(Method((*mock), get_data)).AlwaysReturn(r::error_code::success);
  Fake(Dtor((*mock)));

  return mock;
}

std::unique_ptr<fakeit::Mock<m::i_model>> get_mock_model() {
  auto mock = std::unique_ptr<fakeit::Mock<m::i_model>>(
    new fakeit::Mock<m::i_model>());

  When(Method((*mock), update)).AlwaysReturn(r::error_code::success);
  When(Method((*mock), choose_rank)).AlwaysReturn(r::error_code::success);
  Fake(Dtor((*mock)));

  return mock;
}

std::unique_ptr<r::sender_factory_t> get_mock_sender_factory(fakeit::Mock<r::i_sender>* mock_observation_sender, fakeit::Mock<r::i_sender>* mock_interaction_sender) {
  auto factory = std::unique_ptr<r::sender_factory_t>(
    new r::sender_factory_t());
  factory->register_type(r::value::OBSERVATION_EH_SENDER,
    [mock_observation_sender](r::i_sender** retval, const u::configuration&, r::error_callback_fn* error_callback, r::i_trace*, r::api_status*) { *retval = &mock_observation_sender->get(); return r::error_code::success; });
  factory->register_type(r::value::INTERACTION_EH_SENDER,
    [mock_interaction_sender](r::i_sender** retval, const u::configuration&, r::error_callback_fn* error_callback, r::i_trace*, r::api_status*) { *retval = &mock_interaction_sender->get(); return r::error_code::success; });
  return factory;
}

std::unique_ptr<r::data_transport_factory_t> get_mock_data_transport_factory(fakeit::Mock<m::i_data_transport>* mock_data_transport) {
  auto factory = std::unique_ptr<r::data_transport_factory_t>(
    new r::data_transport_factory_t());
  factory->register_type(r::value::AZURE_STORAGE_BLOB,
    [mock_data_transport](m::i_data_transport** retval, const u::configuration&, r::i_trace* trace, r::api_status*) { *retval = &mock_data_transport->get(); return r::error_code::success; });
  return factory;
}


std::unique_ptr<r::model_factory_t> get_mock_model_factory(fakeit::Mock<m::i_model>* mock_model) {
  auto factory = std::unique_ptr<r::model_factory_t>(
    new r::model_factory_t());
  factory->register_type(r::value::VW,
    [mock_model](m::i_model** retval, const u::configuration&, r::i_trace* trace, r::api_status*) { *retval = &mock_model->get(); return r::error_code::success; });
  return factory;
}
