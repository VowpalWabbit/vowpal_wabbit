#include "restapi_data_transport.h"
#include <cpprest/http_client.h>
#include "api_status.h"
#include "factory_resolver.h"
#include "err_constants.h"

using namespace web; // Common features like URIs.
using namespace web::http; // Common HTTP functionality
using namespace std::chrono;

namespace reinforcement_learning { namespace model_mangement {

  restapi_data_tranport::restapi_data_tranport(const std::string& url) : _url(url) {
  }

  int restapi_data_tranport::get_data_info(time_t& time_point, int& sz, api_status* status) {
    
    return error_code::success;
  }

  int restapi_data_tranport::get_data(model_data& ret, api_status* status) {

    time_t modified_date;
    int datasz;

    int err = get_data_info(modified_date, datasz, status);
    TRY_OR_RETURN(err);

    return err;
  }

}}