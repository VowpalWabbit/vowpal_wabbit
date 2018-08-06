#pragma once

#include "config_collection.h"
#include "async_batcher.h"
#include "eventhub_client.h"
#include "api_status.h"
#include "../error_callback_fn.h"

namespace reinforcement_learning {
  // This class wraps logging utilities, nicely exposed for the live_model class.
  class event_hub_logger_impl {
  public:
    event_hub_logger_impl(
      const utility::config_collection&,
      const std::string& event_hub_host,
      const std::string& event_hub_key_name,
      const std::string& event_hub_key,
      const std::string& event_hub_name,
      int send_high_watermark,
      int send_batch_interval_ms,
      int send_queue_maxsize,
      error_callback_fn* perror_cb = nullptr);

    int init(api_status* status);
    int append(std::string& data, api_status* status);

  private:
    //clients to send data to the eventhub
    eventhub_client _client;

    //handle batching for the data sent to the eventhub client
    async_batcher<eventhub_client> _batcher;
  };
}

