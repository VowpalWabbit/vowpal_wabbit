#pragma once
#include "model_mgmt.h"
#include "safe_vw.h"
#include "../utility/object_pool.h"

namespace reinforcement_learning { namespace model_management {
  class vw_model : public i_model {
  public:
    vw_model();
    int update(const model_data& data, api_status* status = nullptr) override;
    int choose_rank(uint64_t rnd_seed, const char* features, ranking_response& response, api_status* status = nullptr) override;
  private:
    using vw_ptr = std::shared_ptr<safe_vw>;
    using pooled_vw = utility::pooled_object_guard<safe_vw, safe_vw_factory>;
    utility::object_pool<safe_vw, safe_vw_factory> _vw_pool;
  };
}}
