#pragma once

#include "sender.h"
#include "utility/object_pool.h"

#include "configuration.h"
#include "constants.h"
#include "async_batcher.h"
#include "eventhub_client.h"
#include "api_status.h"
#include "../error_callback_fn.h"
#include "utility/watchdog.h"
#include "ranking_response.h"
#include "ranking_event.h"

namespace reinforcement_learning {
  // This class wraps logging event to event_hub in a generic way that live_model can consume.
  class event_logger {
  public:
    event_logger(
      i_sender* sender,
      int send_high_watermark,
      int send_batch_interval_ms,
      int send_queue_maxsize,
      utility::watchdog& watchdog,
      error_callback_fn* perror_cb = nullptr);

    int init(api_status* status);
  
  protected:
    int append(std::string& data, api_status* status);

  protected:
    bool _initialized = false;

    // Handle batching for the data sent to the eventhub client
    async_batcher _batcher;

    utility::object_pool<utility::data_buffer, utility::buffer_factory> _buffer_pool;
  };

  class interaction_logger : public event_logger {
  public:
    interaction_logger(const utility::configuration& c, i_sender* sender, utility::watchdog& watchdog, error_callback_fn* perror_cb = nullptr)
      : event_logger(
        sender,
        c.get_int(name::INTERACTION_SEND_HIGH_WATER_MARK, 198 * 1024),
        c.get_int(name::INTERACTION_SEND_BATCH_INTERVAL_MS, 1000),
        c.get_int(name::INTERACTION_SEND_QUEUE_MAXSIZE, 100000 * 2),
        watchdog,
        perror_cb)
    {}

    int log(const char* event_id, const char* context, const ranking_response& response, api_status* status);
  };

  class observation_logger : public event_logger {
  public:
    observation_logger(const utility::configuration& c, i_sender* sender, utility::watchdog& watchdog, error_callback_fn* perror_cb = nullptr)
      : event_logger(
        sender,
        c.get_int(name::OBSERVATION_SEND_HIGH_WATER_MARK, 198 * 1024),
        c.get_int(name::OBSERVATION_SEND_BATCH_INTERVAL_MS, 1000),
        c.get_int(name::OBSERVATION_SEND_QUEUE_MAXSIZE, 100000 * 2),
        watchdog,
        perror_cb)
    {}

    template <typename D>
    int log(const char* event_id, D outcome, api_status* status) {
      // Serialize outcome
      utility::pooled_object_guard<utility::data_buffer, utility::buffer_factory> buffer(_buffer_pool, _buffer_pool.get_or_create());
      buffer->reset();
      outcome_event::serialize(*buffer.get(), event_id, outcome);
      auto sbuf = buffer->str();

      // Send the outcome event to the backend
      return append(sbuf, status);
    }
  };
}
