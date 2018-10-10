#pragma once

#include "onlinetrainer.net.native.h"

typedef struct vw_online_trainer_context {

} vw_online_trainer_context_t;

// Global exports
extern "C" {
  API vw_online_trainer_context_t* CreateVWOnlineTrainer();
  API void DeleteVWOnlineTrainer(vw_online_trainer_context_t* vw_online_trainer);

  API int Initialise(vw_online_trainer_context_t* vw_online_trainer, vw_lib::api_status* status = nullptr);
}
