#pragma once
#include "ds_model_mgmt.h"

namespace decision_service { namespace model_mangement {
  class restapi_data_tranport : public i_data_transport {
  public:
    model_data get_data() override;
  };
}}
