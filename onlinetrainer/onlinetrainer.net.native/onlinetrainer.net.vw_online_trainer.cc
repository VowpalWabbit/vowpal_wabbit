#include "onlinetrainer.net.vw_online_trainer.h"

static void pipe_managed_callback(const online_trainer::api_status& status, vw_online_trainer_context_t* context)
{
  auto managed_callback_local = context->callback;
  if (managed_callback_local)
  {
    managed_callback_local(status);
  }
}

API vw_online_trainer_context_t* CreateVWOnlineTrainer(const char* arg)
{
	vw_online_trainer_context_t* context = new vw_online_trainer_context_t;
  context->callback = nullptr;
  context->vw_online_trainer = new online_trainer::online_trainer(arg);
	return context;
}

API void DeleteVWOnlineTrainer(vw_online_trainer_context_t* context)
{
  context->callback = nullptr;
  delete context->vw_online_trainer;
	delete context;
}
