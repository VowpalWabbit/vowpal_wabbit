#pragma once

#include "factory_resolver.h"
#include "sender.h"

#include <fakeit/fakeit.hpp>

#include <memory>

std::unique_ptr<fakeit::Mock<reinforcement_learning::i_sender>> get_mock_sender();
std::unique_ptr<fakeit::Mock<reinforcement_learning::model_management::i_data_transport>> get_mock_data_transport();
std::unique_ptr<fakeit::Mock<reinforcement_learning::model_management::i_model>> get_mock_model();

std::unique_ptr<reinforcement_learning::sender_factory_t> get_mock_sender_factory(fakeit::Mock<reinforcement_learning::i_sender>* mock_observation_sender,
  fakeit::Mock<reinforcement_learning::i_sender>* mock_interaction_sender);
std::unique_ptr<reinforcement_learning::data_transport_factory_t> get_mock_data_transport_factory(fakeit::Mock<reinforcement_learning::model_management::i_data_transport>* mock_data_transport);
std::unique_ptr<reinforcement_learning::model_factory_t> get_mock_model_factory(fakeit::Mock<reinforcement_learning::model_management::i_model>* mock_model);