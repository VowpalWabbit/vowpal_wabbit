#pragma once

#include "vw.net.native.h"
#include "vw/common/vw_exception.h"
#include "vw/core/api_status.h"

// Global exports
extern "C"
{
  API const char* NativeExceptionWhat(std::exception const* exception);
  API const char* NativeVwExceptionWhere(VW::vw_exception const* exception, int& line_number);
}
