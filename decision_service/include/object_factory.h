#pragma once
#include "config_collection.h"
#include "api_status.h"
#include "err_constants.h"
#include <functional>

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
            RETURN_ERROR(status, create_fn_exception) << e.what();
          }
          catch(...) {
            // Create functions should not throw. However registered function might be a user defined function
            RETURN_ERROR(status, create_fn_exception) << " Unknown error.";
          }
        }
        // No matching create function
        RETURN_ERROR(status, type_not_registered);
      }

    private:
      std::unordered_map<std::string, create_fn> _creators;
    };
}
}
