#include "http_helper.h"

using namespace web::http; // Common HTTP functionality

namespace reinforcement_learning { namespace utility {

  client::http_client_config get_http_config(){
    client::http_client_config cfg;
    cfg.set_validate_certificates(false);
    return cfg; 
  }

}}