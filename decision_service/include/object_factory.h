#pragma once
#include "config_collection.h"
#include "api_status.h"
#include <functional>
#include "err_constants.h"

namespace reinforcement_learning { namespace utility
{
    template<class I>
    struct object_factory
    {
      using create_fn = std::function<int (I** retval, const config_collection&, api_status* status)>;

      void register_type(const std::string& name, create_fn fptr) { _creators[name] = fptr; }

      int create(I** retval, const std::string& name, const config_collection& cc,api_status* status = nullptr) {
        auto it = _creators.find(name);

        if ( it != _creators.end() ) {
          try {
            return ( it->second )( retval, cc, status );
          }
          catch ( const std::exception& e ) {
            // Create functions should not throw. However registered function might be a user defined function
            return report_error(status, error_code::create_fn_exception, error_code::create_fn_exception_s, e.what());
          }
          catch(...) {
            // Create functions should not throw. However registered function might be a user defined function
            return report_error(status, error_code::create_fn_exception, error_code::create_fn_exception_s, " Unknown error.");
          }
        }

        // No matching create function
        api_status::try_update(status, error_code::type_not_registered, error_code::type_not_registered_s);
        return error_code::type_not_registered;
      }

    private:
      std::unordered_map<std::string, create_fn> _creators;
    };
}
}
