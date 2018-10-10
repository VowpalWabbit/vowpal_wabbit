#include "onlinetrainer.net.vw_online_trainer.h"

API vw_online_trainer_context_t* CreateVWOnlineTrainer()
{
	vw_online_trainer_context_t* context = new vw_online_trainer_context_t;
	return context;
}

API void DeleteVWOnlineTrainer(vw_online_trainer_context_t* context)
{
	delete context;
}

API int Initialise(vw_lib::vw_online_trainer* vw_online_trainer, vw_lib::api_status* status)
{
	return vw_online_trainer->init(status);
}
