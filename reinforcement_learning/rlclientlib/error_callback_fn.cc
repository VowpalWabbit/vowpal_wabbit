#include "error_callback_fn.h"

using namespace std;

namespace reinforcement_learning
{
  void error_callback_fn::set(error_fn fn, void* context)
  {
    lock_guard<mutex> lock(_mutex);
    _fn = fn;
    _context = context;
  }

  void error_callback_fn::report_error(api_status& s)
  {
    if (_fn == nullptr)
      return;

    lock_guard<mutex> lock(_mutex);
    if (_fn != nullptr)
    {
      try
      {
        _fn(s, _context);
      }
      catch (...)
      {
        // Error handler is throwing so can't call it again
      }
    }
  }

  error_callback_fn::error_callback_fn(error_fn fn, void* cntxt)
    : _fn(fn), _context(cntxt)
  {
  }
}
