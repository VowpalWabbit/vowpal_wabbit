/**
* @brief Online Trainer API definition.
*
* @file online_trainer.h
*/
#pragma once
#include "vw_settings.h"
#include "api_status.h"

namespace online_trainer {

  class online_trainer {

  public:
    vw_settings *m_settings;
    vw_model *m_model;
    explicit online_trainer(const char *arg, uint8_t* model, size_t l);
  };
}
