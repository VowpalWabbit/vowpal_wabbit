#include "bg_model_download.h"
#include "api_status.h"

namespace reinforcement_learning { namespace model_management {
  bg_model_download::bg_model_download(i_data_transport* ptrans, data_callback_fn* pdata_cb)
    : _ptrans(ptrans), _pdata_cb(pdata_cb){}

  bg_model_download::~bg_model_download() { delete _ptrans; }

  bg_model_download::bg_model_download(bg_model_download&& temp) noexcept {
    _ptrans = temp._ptrans;
    temp._ptrans = nullptr;
    _pdata_cb = temp._pdata_cb;
    temp._pdata_cb = nullptr;
  }

  bg_model_download& bg_model_download::operator=(bg_model_download&& temp) noexcept {
    if (&temp != this) {
      const auto x = _ptrans;
      _ptrans      = temp._ptrans;
      temp._ptrans = x;
    }
    return *this;
  }

  int bg_model_download::run_once(api_status* status) const {
    model_data md;
    const auto scode = _ptrans->get_data(md, status);
    TRY_OR_RETURN(scode);
    return _pdata_cb->report_data(md, status);
  }
}}
