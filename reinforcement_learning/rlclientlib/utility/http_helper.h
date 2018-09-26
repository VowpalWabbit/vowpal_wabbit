#pragma once
#include <cpprest/http_client.h>

namespace reinforcement_learning { namespace utility {

web::http::client::http_client_config get_http_config();

}}