#include "error_callback_fn.h"

using namespace std;
namespace decision_service {

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
      catch(...)
      {
        // What to do if the error handler throws?
      }
    }
  }

  error_callback_fn::error_callback_fn(error_fn fn, void* cntxt) 
  : _fn(fn), _context(cntxt)
  {
  }
}
