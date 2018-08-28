#include "event_hub_logger.h"

#include "err_constants.h"

namespace reinforcement_learning
{
  event_hub_logger::event_hub_logger(
    const utility::configuration& c,
    const std::string& event_hub_host,
    const std::string& event_hub_key_name,
    const std::string& event_hub_key,
    const std::string& event_hub_name,
    int send_high_watermark,
    int send_batch_interval_ms,
    int send_queue_maxsize,
    error_callback_fn* perror_cb
  )
    : _client(
        event_hub_host,
        event_hub_key_name,
        event_hub_key,
        event_hub_name,
        c.get_bool(name::EH_TEST, false)),
      _batcher(
        _client,
        perror_cb,
        send_high_watermark,
        send_batch_interval_ms,
        send_queue_maxsize)
  {}

  int event_hub_logger::init(api_status* status) {
    RETURN_IF_FAIL(_batcher.init(status));
    RETURN_IF_FAIL(_client.init(status));
    _initialized = true;
    return error_code::success;
  }

  int event_hub_logger::v_append(std::string& item, api_status* status) {
    if(!_initialized) {
      api_status::try_update(status, error_code::not_initialized,
        "Logger not initialized. Call init() first.");
      return error_code::not_initialized;
    }

    // Add item to the batch (will be sent later)
    return _batcher.append(item, status);
  }
}
