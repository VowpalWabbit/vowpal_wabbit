#pragma once

#include "logger.h"

#include "configuration.h"
#include "constants.h"
#include "async_batcher.h"
#include "eventhub_client.h"
#include "api_status.h"
#include "../error_callback_fn.h"

namespace reinforcement_learning {
  // This class wraps logging event to event_hub in a generic way that live_model can consume.
  class event_hub_logger : public i_logger {
  public:
    event_hub_logger(
      const utility::configuration&,
      const std::string& event_hub_host,
      const std::string& event_hub_key_name,
      const std::string& event_hub_key,
      const std::string& event_hub_name,
      int send_high_watermark,
      int send_batch_interval_ms,
      int send_queue_maxsize,
      error_callback_fn* perror_cb = nullptr);

    virtual int init(api_status* status) override;
  protected:
    virtual int v_append(std::string& data, api_status* status) override;

  private:
    bool _initialized = false;

    //clients to send data to the eventhub
    eventhub_client _client;

    //handle batching for the data sent to the eventhub client
    async_batcher<eventhub_client> _batcher;
  };

  class event_hub_observation_logger : public event_hub_logger {
  public:
    event_hub_observation_logger(const utility::configuration& c, error_callback_fn* perror_cb = nullptr)
      : event_hub_logger(
        c,
        c.get(name::OBSERVATION_EH_HOST, "localhost:8080"),
        c.get(name::OBSERVATION_EH_KEY_NAME, ""),
        c.get(name::OBSERVATION_EH_KEY, ""),
        c.get(name::OBSERVATION_EH_NAME, "interaction"),
        c.get_int(name::OBSERVATION_SEND_HIGH_WATER_MARK, 198 * 1024),
        c.get_int(name::OBSERVATION_SEND_BATCH_INTERVAL_MS, 1000),
        c.get_int(name::OBSERVATION_SEND_QUEUE_MAXSIZE, 10000 * 2),
        perror_cb)
    {}
  };

  class event_hub_interaction_logger : public event_hub_logger {
  public:
    event_hub_interaction_logger(const utility::configuration& c, error_callback_fn* perror_cb = nullptr)
      : event_hub_logger(
        c,
        c.get(name::INTERACTION_EH_HOST, "localhost:8080"),
        c.get(name::INTERACTION_EH_KEY_NAME, ""),
        c.get(name::INTERACTION_EH_KEY, ""),
        c.get(name::INTERACTION_EH_NAME, "interaction"),
        c.get_int(name::INTERACTION_SEND_HIGH_WATER_MARK, 198 * 1024),
        c.get_int(name::INTERACTION_SEND_BATCH_INTERVAL_MS, 1000),
        c.get_int(name::INTERACTION_SEND_QUEUE_MAXSIZE, 10000 * 2),
        perror_cb)
    {}
  };
}
