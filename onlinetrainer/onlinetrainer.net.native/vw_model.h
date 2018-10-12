#pragma once
#include "vw_settings.h"

namespace online_trainer {
  class vw_settings;
  class vw_model {
  public:
    vw_model(vw_settings *settings);
    vw_model(const char *args);
  };
}
