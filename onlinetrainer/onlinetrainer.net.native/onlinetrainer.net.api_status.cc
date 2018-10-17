#include "onlinetrainer.net.api_status.h"

API online_trainer::api_status* CreateApiStatus()
{
  return new online_trainer::api_status();
}

API void DeleteApiStatus(online_trainer::api_status* status)
{
  delete status;
}

API const char* GetApiStatusErrorMessage(online_trainer::api_status* status)
{
  return status->get_error_msg();
}

API int GetApiStatusErrorCode(online_trainer::api_status* status)
{
  return status->get_error_code();
}
