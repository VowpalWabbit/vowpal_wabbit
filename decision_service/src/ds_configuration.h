#pragma once

#include <string>

namespace decision_service {

  class configuration {

  public:
    const std::string & app_id() const;

    //model config
    const std::string & model_url() const;
    const int model_refresh_period_ms() const;

    //eventhub config
    const size_t batch_max_size() const;
    const int batch_timeout_ms() const;
    const size_t queue_max_size() const;
    const std::string & eventhub_host() const;
    const std::string & shared_access_key_name() const;
    const std::string & shared_access_key() const;
    const std::string & eventhub_interaction_name() const;
    const std::string & eventhub_observation_name() const;

    configuration();//default values
    configuration(const std::string& json);//deserialize the json string


  private:
    std::string _app_id;
    std::string _model_url;
    int _model_refresh_period_ms;
    std::string _eventhub_interaction_name;
    std::string _eventhub_observation_name;
    std::string _eventhub_host;
    std::string _shared_access_key_name;
    std::string _shared_access_key;
    size_t _batch_max_size;
    size_t _queue_max_size;
    int _batch_timeout_ms;
  };
}