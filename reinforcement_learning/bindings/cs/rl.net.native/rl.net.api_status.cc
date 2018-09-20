#include "rl.net.api_status.h"

API r::api_status* CreateApiStatus()
{
    return new r::api_status();
}

API void DeleteApiStatus(r::api_status* status)
{
    delete status;
}

API const char* GetApiStatusErrorMessage(r::api_status* status)
{
    return status->get_error_msg();
}

API int GetApiStatusErrorCode(r::api_status* status)
{
    return status->get_error_code();
}