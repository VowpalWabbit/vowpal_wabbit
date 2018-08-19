#pragma once
#include "configuration.h"
#include "api_status.h"
#include "err_constants.h"
#include <functional>

namespace reinforcement_learning { namespace utility
{
    template<class I, typename ...Args>
    struct object_factory
    {
      using create_fn = std::function<int (I** retval, Args&& ...args, api_status* status)>;

      void register_type(const std::string& name, create_fn fptr) { _creators[name] = fptr; }

      // There is a compiler bug in MSVC where a parameter pack cannot be followed by a default argument,
      // so an overload is used to get around this.
      int create(I** retval, const std::string& name, Args&& ...args) {
        return create(retval, name, args..., nullptr);
      }

      int create(I** retval, const std::string& name, Args&& ...args, api_status* status) {
        auto it = _creators.find(name);

        if ( it != _creators.end() ) {
          try {
            return ( it->second )( retval, std::forward<Args>(args)..., status );
          }
          catch ( const std::exception& e ) {
            // Create functions should not throw. However registered function might be a user defined function
            RETURN_ERROR_LS(status, create_fn_exception) << e.what();
          }
          catch(...) {
            // Create functions should not throw. However registered function might be a user defined function
            RETURN_ERROR_LS(status, create_fn_exception) << " Unknown error.";
          }
        }
        // No matching create function
        RETURN_ERROR_LS(status, type_not_registered);
      }

    private:
      std::unordered_map<std::string, create_fn> _creators;
    };
}
}
