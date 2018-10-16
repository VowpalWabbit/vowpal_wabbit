#pragma once
#include "vw_settings.h"
#include "vw_base.h"

namespace online_trainer {
  class vw_settings;
  class vw_model : public vw_base {
  public:
    vw_model(vw_settings *settings);
    vw_model(const char *args);
  };
}
