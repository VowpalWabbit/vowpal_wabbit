#include "logger.h"
#include "err_constants.h"
#include "constants.h"

#include "logger_impl.h"

namespace reinforcement_learning
{
  event_hub_logger::event_hub_logger(
    const utility::config_collection& c,
    const std::string& event_hub_host,
    const std::string& event_hub_key_name,
    const std::string& event_hub_key,
    const std::string& event_hub_name,
    int send_high_watermark,
    int send_batch_interval_ms,
    int send_queue_maxsize,
    error_callback_fn* perror_cb)
  {
    _pimpl = std::unique_ptr<event_hub_logger_impl>(new event_hub_logger_impl(
      c,
      event_hub_host,
      event_hub_key_name,
      event_hub_key,
      event_hub_name,
      send_high_watermark,
      send_batch_interval_ms,
      send_queue_maxsize,
      perror_cb));
  }

  event_hub_logger::~event_hub_logger() = default;

  int event_hub_logger::init(api_status* status) {
    return _pimpl->init(status);
  }

  int event_hub_logger::v_append(std::string& item, api_status* status)
  {
    return _pimpl->append(item, status);
  }
}