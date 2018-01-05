/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/
#include "ds_api.h"

#include <cpprest/http_client.h>
#include <memory>

namespace Microsoft {
  namespace DecisionService {

    using namespace std;
    using namespace utility;
    using namespace web::http;
    using namespace web::http::client;

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
      config.model_url = conversions::to_utf8string(json[U("ModelBlobUri")].as_string());
      config.eventhub_interaction_connection_string = conversions::to_utf8string(json[U("EventHubInteractionConnectionString")].as_string());
      config.eventhub_observation_connection_string = conversions::to_utf8string(json[U("EventHubObservationConnectionString")].as_string());
      
      return config;
    }

    DecisionServiceConfiguration::DecisionServiceConfiguration()
      : certificate_validation_enabled(true),
      // with event sizes of 3kb & 5s batching, 2 connections can get 50Mbps
      num_parallel_connection(2),
      batching_timeout_in_milliseconds(5 * 1000),
      batching_queue_max_size(8 * 1024)
    { }

    //void DecisionServiceConfiguration::set_listener(DecisionServiceListener* listener)
    //{
    //  printf("ptr registered: %p\n", listener);
    //  _listener = shared_ptr<DecisionServiceListener>(listener);
    //}
  }
}
