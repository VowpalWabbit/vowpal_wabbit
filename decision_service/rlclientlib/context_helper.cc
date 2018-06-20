#include <cpprest/json.h>
#include <object_factory.h>
#include <cpprest/asyncrt_utils.h>
#include "err_constants.h"

namespace sutil = ::utility::conversions;

namespace reinforcement_learning { namespace utility {
  const auto multi = sutil::to_string_t("_multi");
  /**
   * \brief Get the number of actions found in context json string.  Actions should be in an array
   * called _multi under the root name space.  {_multi:[{"name1":"val1"},{"name1":"val1"}]}
   * 
   * \param count   : Return value passed in as a reference.
   * \param context : String with context json
   * \param status  : Pointer to api_status object that contains an error code and error description in
   *                  case of failure
   * \return  error_code::success if there are no errors.  If there are errors then the error code is
   *          returned. 
   */
  int get_action_count(size_t& count, const char *context, api_status* status) {
    try {
      const auto scontext = sutil::to_string_t(std::string(context));
      auto json_obj = web::json::value::parse(scontext);
      if ( json_obj.has_array_field(multi) ) {
        auto const arr = json_obj.at(multi).as_array();
        count = arr.size();
        if ( count > 0 )
          return reinforcement_learning::error_code::success;
        RETURN_ERROR(status, json_no_actions_found);
      }
      RETURN_ERROR(status, json_no_actions_found);
    }
    catch ( const std::exception& e ) {
      RETURN_ERROR(status, json_parse_error) << e.what();
    }
    catch ( ... ) {
      RETURN_ERROR(status, json_parse_error) << error_code::unkown_s;
    }
  }
}}
