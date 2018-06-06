#include "vw_model.h"
#include "err_constants.h"

namespace reinforcement_learning { namespace model_management {
  int vw_model::init(model_data& data, api_status* status) { return error_code::success; }
  int vw_model::choose_rank(char* features, int actions[], api_status* status) { return error_code::success; }
}}
