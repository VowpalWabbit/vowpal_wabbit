#pragma once
#include "configuration.h"
#include "api_status.h"
#include "err_constants.h"
#include "trace_logger.h"
#include <functional>

namespace reinforcement_learning { namespace utility
{
    template<class I, typename ...Args>
    struct object_factory
    {
      using create_fn = std::function<int (I** retval, Args&& ...args, i_trace* trace_logger, api_status* status)>;

      void register_type(const std::string& name, create_fn fptr) { _creators[name] = fptr; }

      // There is a compiler bug in MSVC where a parameter pack cannot be followed by a default argument,
      // so an overload is used to get around this.
      // Both trace_logger and api_status are default
      int create(I** retval, const std::string& name, Args&& ...args) {
        return create(retval, name, std::forward<Args>(args)..., nullptr, nullptr);
      }
      // Only api_status is default
      int create(I** retval, const std::string& name, Args&& ...args, i_trace* trace) {
        return create(retval, name, std::forward<Args>(args)..., trace, nullptr);
      }
      // Only trace_logger is default
      int create(I** retval, const std::string& name, Args&& ...args, api_status* status) {
        return create(retval, name, std::forward<Args>(args)..., nullptr, status);
      }

      int create(I** retval, const std::string& name, Args&& ...args, i_trace* trace, api_status* status) {
        auto it = _creators.find(name);

        if ( it != _creators.end() ) {
          try {
            return ( it->second )( retval, std::forward<Args>(args)..., trace, status );
          }
          catch ( const std::exception& e ) {
            // Create functions should not throw. However registered function might be a user defined function
            RETURN_ERROR_LS(trace, status, create_fn_exception) << e.what();
          }
          catch(...) {
            // Create functions should not throw. However registered function might be a user defined function
            RETURN_ERROR_LS(trace, status, create_fn_exception) << " Unknown error.";
          }
        }
        // No matching create function
        RETURN_ERROR_LS(trace, status, type_not_registered);
      }

    private:
      std::unordered_map<std::string, create_fn> _creators;
    };
}
}
