/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/
#include "ds_api.h"

#include <cpprest/http_client.h>
#include <memory>
#include <stdexcept>

namespace Microsoft {
  namespace DecisionService {

    using namespace std;
    using namespace utility;
    using namespace web::http;
    using namespace web::http::client;

    std::string to_string(web::json::value json, string_t property)
    {
      auto val = json[property];
      if (val.is_null() || !val.is_string())
      {
        ostringstream msg;
        msg << "JSON property: '" << property << "' not found";
        throw domain_error(msg.str());
      }

      return conversions::to_utf8string(val.as_string());
    }

    DecisionServiceConfiguration DecisionServiceConfiguration::Download(const char* url/*, bool certificate_validation_enabled*/) throw(std::exception)
    {
      // mainly for unit tests
      http_client_config http_config;
      //http_config.set_validate_certificates(certificate_validation_enabled);

      // download configuration
      http_client client(conversions::to_string_t(url), http_config);
      auto json = client
        .request(methods::GET)
        .then([=](http_response response) { return response.extract_json(/* ignore content type */ true); })
        .get();

      DecisionServiceConfiguration config;
      config.model_url = to_string(json, U("ModelBlobUri"));
      config.eventhub_interaction_connection_string = to_string(json ,U("EventHubInteractionConnectionString"));
      config.eventhub_observation_connection_string = to_string(json, U("EventHubObservationConnectionString"));
      config.app_id = to_string(json, U("ApplicationID"));

      return config;
    }

    DecisionServiceConfiguration::DecisionServiceConfiguration()
      : certificate_validation_enabled(true),
      // with event sizes of 3kb & 5s batching, 2 connections can get 50Mbps
      num_parallel_connection(2),
      batching_timeout_in_milliseconds(5 * 1000),
      batching_queue_max_size(8 * 1024),
      log_level(DecisionServiceLogLevel::error),
      explorer(nullptr),
      logger(nullptr)
    { }

    bool DecisionServiceConfiguration::can_log(DecisionServiceLogLevel level_of_message)
    {
      return logger && level_of_message <= log_level;
    }
  }
}
