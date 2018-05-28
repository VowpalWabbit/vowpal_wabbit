#pragma once
#include "model_mgmt.h"
#include <chrono>
#include <string>

namespace reinforcement_learning { namespace model_mangement {
  class restapi_data_tranport : public i_data_transport {
  public:
    restapi_data_tranport(const std::string& url);
    int get_data(model_data& data, api_status* status) override;

  private:
    using time_t = std::chrono::time_point<std::chrono::system_clock>;
    int get_data_info(time_t& time_point, int& sz, api_status* status);
    std::string _url;
  };
}}
