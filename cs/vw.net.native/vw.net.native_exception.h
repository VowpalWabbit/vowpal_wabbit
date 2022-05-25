#pragma once

#include "vw.net.native.h"
#include "api_status.h"
#include "vw_exception.h"

// Global exports
extern "C" {
  API const char* NativeExceptionWhat(std::exception const* exception);
  API const char* NativeVwExceptionWhere(VW::vw_exception const* exception, int& line_number);
}