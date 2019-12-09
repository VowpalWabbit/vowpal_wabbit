// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw_exception.h"

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

namespace VW
{
vw_exception::vw_exception(const char* pfile, int plineNumber, std::string const& pmessage) noexcept
    : file(pfile), message(pmessage), lineNumber(plineNumber)
{
}

vw_exception::vw_exception(const vw_exception& ex) noexcept
    : file(ex.file), message(ex.message), lineNumber(ex.lineNumber)
{
}

vw_exception& vw_exception::operator=(const vw_exception& other) noexcept
{
  // check for self-assignment
  if (&other == this)
    return *this;

  file = other.file;
  message = other.message;
  lineNumber = other.lineNumber;

  return *this;
}

vw_exception::~vw_exception() noexcept = default;

const char* vw_exception::what() const noexcept { return message.c_str(); }

const char* vw_exception::Filename() const { return file; }

int vw_exception::LineNumber() const { return lineNumber; }

#ifdef _WIN32

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

struct StopWatchData
{
  LARGE_INTEGER frequency_;
  LARGE_INTEGER startTime_;
};

StopWatch::StopWatch() : data(new StopWatchData())
{
  if (!::QueryPerformanceFrequency(&data->frequency_))
    THROW("Error with QueryPerformanceFrequency");
  ::QueryPerformanceCounter(&data->startTime_);
}

StopWatch::~StopWatch() { delete data; }

double StopWatch::MilliSeconds() const
{
  LARGE_INTEGER now;
  ::QueryPerformanceCounter(&now);

  return double(now.QuadPart - data->startTime_.QuadPart) / (double(data->frequency_.QuadPart) / 1000);
}

bool launchDebugger()
{
  // Get System directory, typically c:\windows\system32
  std::wstring systemDir(MAX_PATH + 1, '\0');
  UINT nChars = GetSystemDirectoryW(&systemDir[0], (UINT)systemDir.length());
  if (nChars == 0)
    return false;  // failed to get system directory
  systemDir.resize(nChars);

  // Get process ID and create the command line
  DWORD pid = GetCurrentProcessId();
  std::wostringstream s;
  s << systemDir << L"\\vsjitdebugger.exe -p " << pid;
  std::wstring cmdLine = s.str();

  // Start debugger process
  STARTUPINFOW si;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);

  PROCESS_INFORMATION pi;
  ZeroMemory(&pi, sizeof(pi));

  if (!CreateProcessW(NULL, &cmdLine[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    return false;

  // Close debugger process handles to eliminate resource leak
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);

  // Wait for the debugger to attach
  while (!IsDebuggerPresent()) Sleep(100);

  // Stop execution so the debugger can take over
  DebugBreak();
  return true;
}
#endif
}  // namespace VW
