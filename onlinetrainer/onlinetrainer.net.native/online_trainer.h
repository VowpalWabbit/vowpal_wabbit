/**
* @brief Online Trainer API definition.
*
* @file online_trainer.h
*/
#pragma once

// #include "../../vowpalwabbit/vw.h"
#include "vw_settings.h"

namespace online_trainer {

  class online_trainer {

  public:
    vw_settings *m_settings;
    vw_model *m_model;
    // vw *m_vw;
    explicit online_trainer(
      const char *arg);
  };
}
