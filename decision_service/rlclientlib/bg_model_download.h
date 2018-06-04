#pragma once
#include "restapi_data_transport.h"
#include "data_callback_fn.h"
namespace reinforcement_learning {
  class error_callback_fn;
}

namespace reinforcement_learning { namespace model_management {
  class bg_model_download {
  public:
    bg_model_download(i_data_transport* ptrans, data_callback_fn* pdata_cb);
    ~bg_model_download();
    bg_model_download(bg_model_download&& temp) noexcept;
    bg_model_download& operator=(bg_model_download&& temp) noexcept;

    int run_once(api_status* status) const;
  private:
    i_data_transport* _ptrans;
    data_callback_fn* _pdata_cb;
  };
}}
