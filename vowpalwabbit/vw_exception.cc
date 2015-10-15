#include "vw_exception.h"

namespace VW {

  vw_exception::vw_exception(const char* pfile, int plineNumber, std::string pmessage)
    : file(pfile), message(pmessage), lineNumber(plineNumber)
  {
  }

  vw_exception::vw_exception(const vw_exception& ex)
    : file(ex.file), message(ex.message), lineNumber(ex.lineNumber)
  {
  }

  vw_exception::~vw_exception() _NOEXCEPT
  {
  }

  const char* vw_exception::what() const _NOEXCEPT
  {
    return message.c_str();
  }

  const char* vw_exception::Filename() const
  {
    return file;
  }

  int vw_exception::LineNumber() const
  {
    return lineNumber;
  }

#ifdef _WIN32
#include <Windows.h>

  void vw_trace(const char* filename, int linenumber, const char* fmt, ...)
  {
    char buffer[4 * 1024];
    int offset = sprintf_s(buffer, sizeof(buffer), "%s:%d (%d): ", filename, linenumber, GetCurrentThreadId());

    va_list argptr;
    va_start(argptr, fmt);
    offset += vsprintf_s(buffer + offset, sizeof(buffer) - offset, fmt, argptr);
    va_end(argptr);

    sprintf_s(buffer + offset, sizeof(buffer) - offset, "\n");

    OutputDebugStringA(buffer);
  }
#endif
}
