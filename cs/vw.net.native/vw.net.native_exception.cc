#include "vw.net.native_exception.h"

API const char* NativeExceptionWhat(std::exception const* exception) { return exception->what(); }

API const char* NativeVwExceptionWhere(VW::vw_exception const* exception, int& line_number)
{
  line_number = exception->line_number();
  return exception->filename();
}