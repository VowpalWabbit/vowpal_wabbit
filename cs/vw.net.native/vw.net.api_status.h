#pragma once

#include "vw.net.native.h"
#include "vw/core/api_status.h"

// Global exports
extern "C"
{
  // NOTE: THIS IS NOT POLYMORPHISM SAFE!
  API VW::experimental::api_status* CreateApiStatus();
  API void DeleteApiStatus(VW::experimental::api_status* status);

  // TODO: We should think about how to avoid extra string copies; ideally, err constants
  // should be able to be shared between native/managed, but not clear if this is possible
  // right now. (SourceGenerator?)
  API const char* GetApiStatusErrorMessage(VW::experimental::api_status* status);
  API int GetApiStatusErrorCode(VW::experimental::api_status* status);

  // API void UpdateApiStatusSafe(reinforcement_learning::api_status* status, int error_code, const char* message);
  // API void ClearApiStatusSafe(reinforcement_learning::api_status* status);
}
