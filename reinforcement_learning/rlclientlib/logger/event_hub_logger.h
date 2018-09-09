#pragma once

#include "logger.h"

#include "configuration.h"
#include "constants.h"
#include "async_batcher.h"
#include "eventhub_client.h"
#include "api_status.h"
#include "../error_callback_fn.h"
#include "utility/watchdog.h"

namespace reinforcement_learning {
  // This class wraps logging event to event_hub in a generic way that live_model can consume.
  class event_hub_logger {
  public:
    event_hub_logger(
      i_logger* logger,
      int send_high_watermark,
      int send_batch_interval_ms,
      int send_queue_maxsize,
      utility::watchdog& watchdog,
      error_callback_fn* perror_cb = nullptr);

    int init(api_status* status);
  
  public:
    int append(std::string& data, api_status* status);

  private:
    bool _initialized = false;

    // Clients to send data to the eventhub
    std::unique_ptr<i_logger> _client;

    // Handle batching for the data sent to the eventhub client
    async_batcher _batcher;
  };

  class observation_logger : public event_hub_logger {
  public:
    observation_logger(const utility::configuration& c, i_logger* logger, utility::watchdog& watchdog, error_callback_fn* perror_cb = nullptr)
      : event_hub_logger(
        logger,
        c.get_int(name::OBSERVATION_SEND_HIGH_WATER_MARK, 198 * 1024),
        c.get_int(name::OBSERVATION_SEND_BATCH_INTERVAL_MS, 1000),
        c.get_int(name::OBSERVATION_SEND_QUEUE_MAXSIZE, 100000 * 2),
        watchdog,
        perror_cb)
    {}
  };

  class interaction_logger : public event_hub_logger {
  public:
    interaction_logger(const utility::configuration& c, i_logger* logger, utility::watchdog& watchdog, error_callback_fn* perror_cb = nullptr)
      : event_hub_logger(
        logger,
        c.get_int(name::INTERACTION_SEND_HIGH_WATER_MARK, 198 * 1024),
        c.get_int(name::INTERACTION_SEND_BATCH_INTERVAL_MS, 1000),
        c.get_int(name::INTERACTION_SEND_QUEUE_MAXSIZE, 100000 * 2),
        watchdog,
        perror_cb)
    {}
  };
}
