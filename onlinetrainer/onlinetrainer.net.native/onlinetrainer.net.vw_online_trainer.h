#pragma once

#include "onlinetrainer.net.native.h"
typedef void(*managed_callback_t)(const online_trainer::api_status&);

typedef struct vw_online_trainer_context {
  online_trainer::online_trainer* vw_online_trainer;
  managed_callback_t callback;
} vw_online_trainer_context_t;

// Global exports
extern "C" {
  API vw_online_trainer_context_t* CreateVWOnlineTrainer(const char* arg);
  API void DeleteVWOnlineTrainer(vw_online_trainer_context_t* vw_online_trainer);

  API void VWOnlineTrainerSetCallback(vw_online_trainer_context_t* vw_online_trainer, managed_callback_t callback = nullptr);
}
