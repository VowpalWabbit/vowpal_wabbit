#pragma once
#include "restapi_data_transport.h"
#include "data_callback_fn.h"
namespace reinforcement_learning {
  class error_callback_fn;
}

namespace reinforcement_learning { namespace model_management {
  class model_download {
  public:
    model_download(i_data_transport* ptrans, data_callback_fn* pdata_cb);
    model_download(model_download&& temp) noexcept;
    model_download& operator=(model_download&& temp) noexcept;

    int run_once(api_status* status) const;
  private:
    // Lifetime of pointers managed by user of this class
    i_data_transport* _ptrans;
    data_callback_fn* _pdata_cb;
  };
}}
