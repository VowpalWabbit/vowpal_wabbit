#pragma once
#include "model_mgmt/model_mgmt.h"

namespace reinforcement_learning { namespace model_mangement {
  class restapi_data_tranport : public i_data_transport {
  public:
    model_data get_data() override;
  };
}}
