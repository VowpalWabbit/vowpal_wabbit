#pragma once

#include "factory_resolver.h"
#include "logger.h"

#include <fakeit/fakeit.hpp>

#include <memory>

std::unique_ptr<fakeit::Mock<reinforcement_learning::i_logger>> get_mock_logger();
std::unique_ptr<fakeit::Mock<reinforcement_learning::model_management::i_data_transport>> get_mock_data_transport();
std::unique_ptr<fakeit::Mock<reinforcement_learning::model_management::i_model>> get_mock_model();

std::unique_ptr<reinforcement_learning::logger_factory_t> get_mock_logger_factory(fakeit::Mock<reinforcement_learning::i_logger>* mock_observation_logger,
  fakeit::Mock<reinforcement_learning::i_logger>* mock_interaction_logger);
std::unique_ptr<reinforcement_learning::data_transport_factory_t> get_mock_data_transport_factory(fakeit::Mock<reinforcement_learning::model_management::i_data_transport>* mock_data_transport);
std::unique_ptr<reinforcement_learning::model_factory_t> get_mock_model_factory(fakeit::Mock<reinforcement_learning::model_management::i_model>* mock_model);