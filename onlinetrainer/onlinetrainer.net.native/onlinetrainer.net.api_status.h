#pragma once

#include "onlinetrainer.net.native.h"

// Global exports
extern "C" {
    // NOTE: THIS IS NOT POLYMORPHISM SAFE!
    API vw_lib::api_status* CreateApiStatus();
    API void DeleteApiStatus(vw_lib::api_status* status);

    // TODO: We should think about how to avoid extra string copies; ideally, err constants
    // should be able to be shared between native/managed, but not clear if this is possible
    // right now.
    API const char* GetApiStatusErrorMessage(vw_lib::api_status* status);
    API int GetApiStatusErrorCode(vw_lib::api_status* status);
}
