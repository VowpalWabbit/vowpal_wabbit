#include "event_logger.h"
#include "ranking_event.h"
#include "err_constants.h"

namespace reinforcement_learning
{
  namespace u = utility;

  event_logger::event_logger(
    i_sender* sender,
    int send_high_watermark,
    int send_batch_interval_ms,
    int send_queue_maxsize,
    u::watchdog& watchdog,
    error_callback_fn* perror_cb
  )
    : _batcher(
        sender,
        watchdog,
        perror_cb,
        send_high_watermark,
        send_batch_interval_ms,
        send_queue_maxsize),
      _buffer_pool(new u::buffer_factory(u::translate_func('\n', ' ')))
  {}

  int event_logger::init(api_status* status) {
    RETURN_IF_FAIL(_batcher.init(status));
    _initialized = true;
    return error_code::success;
  }

  int event_logger::append(std::string& item, api_status* status) {
    if(!_initialized) {
      api_status::try_update(status, error_code::not_initialized,
        "Logger not initialized. Call init() first.");
      return error_code::not_initialized;
    }

    // Add item to the batch (will be sent later)
    return _batcher.append(item, status);
  }

  int interaction_logger::log(const char* event_id, const char* context, const ranking_response& response, api_status* status) {
    u::pooled_object_guard<u::data_buffer, u::buffer_factory> guard(_buffer_pool, _buffer_pool.get_or_create());
    guard->reset();
    ranking_event::serialize(*guard.get(), event_id, context, response);
    auto sbuf = guard->str();
    return append(sbuf, status);
  }
}
