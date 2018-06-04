#include "data_callback_fn.h"
#include <exception>
#include <object_factory.h>
#include "err_constants.h"

namespace e = reinforcement_learning::error_code;

namespace reinforcement_learning { namespace model_management {
  int model_management::data_callback_fn::report_data(const model_data& data, api_status* status) {
    // need not be thread safe since this is only called from one thread
    try {
      _fn(data, _context);
      return error_code::success;
    }
    catch ( const std::exception& ex ) {
      return report_error(status, 
        e::data_callback_exception, 
        e::data_callback_exception_s, 
        ex.what());
    }
    catch ( ... ) {
      return report_error(status,
        e::data_callback_exception,
        e::data_callback_exception_s,
        "Unknown exception");
    }
  }

  data_callback_fn::data_callback_fn(data_fn fn, void* ctxt)
  : _fn{fn}, _context{ctxt} {}
}}
