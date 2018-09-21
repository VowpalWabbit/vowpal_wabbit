#include "rl.net.api_status.h"

API reinforcement_learning::api_status* CreateApiStatus()
{
    return new reinforcement_learning::api_status();
}

API void DeleteApiStatus(reinforcement_learning::api_status* status)
{
    delete status;
}

API const char* GetApiStatusErrorMessage(reinforcement_learning::api_status* status)
{
    return status->get_error_msg();
}

API int GetApiStatusErrorCode(reinforcement_learning::api_status* status)
{
    return status->get_error_code();
}