#include "async_batcher.h"

namespace reinforcement_learning {
  int async_batcher::init(api_status* status) {
    RETURN_IF_FAIL(_periodic_background_proc.init(this, status));
    return error_code::success;
  }

  int async_batcher::append(std::string&& evt, api_status* status) {
    if (_queue.size() < _queue_max_size) {
      _queue.push(std::move(evt));
      return error_code::success;
    }
    RETURN_ERROR_LS(status, background_queue_overflow) << "Queue size: " << _queue.size() << "Dropped event: " << evt;
  }

  int async_batcher::append(std::string& evt, api_status* status) {
    return append(std::move(evt), status);
  }

  int async_batcher::run_iteration(api_status* status) {
    flush();
    return error_code::success;
  }

  size_t async_batcher::fill_buffer(size_t remaining, std::string& buf_to_send)
  {
    // There is at least one element.  Pop uses move assignment
    // Copy can be avoided if send size is satisfied
    _queue.pop(&buf_to_send);
    auto filled_size = buf_to_send.size();
    --remaining;
    if (remaining <= 0 || filled_size >= _send_high_water_mark) {
      return remaining;
    }

    // Send size not satisfied.  Shift to larger buffer to satisfy send.
    // Copy is needed but reuse existing tmp buffer to avoid allocation
    _buffer.reset();
    _buffer << buf_to_send;

    while (remaining > 0 && filled_size < _send_high_water_mark) {
      _queue.pop(&buf_to_send);
      _buffer << "\n" << buf_to_send;
      --remaining;
      filled_size += buf_to_send.size();
    }
    buf_to_send = std::move(_buffer.str());
    return remaining;
  }

  void async_batcher::flush() {
    const auto queue_size = _queue.size();

    // Early exit if queue is empty.
    if (queue_size == 0) {
      return;
    }

    auto remaining = queue_size;
    std::string buf_to_send;
    // Handle batching
    while (remaining > 0) {
      remaining = fill_buffer(remaining, buf_to_send);
      api_status status;
      if (_logger.append(buf_to_send, &status) != error_code::success) {
        ERROR_CALLBACK(_perror_cb, status);
      }
    }
  }

  async_batcher::async_batcher(i_logger& logger, utility::watchdog& watchdog, error_callback_fn* perror_cb, const size_t send_high_water_mark,
    const size_t batch_timeout_ms, const size_t queue_max_size)
    : _logger(logger),
    _send_high_water_mark(send_high_water_mark),
    _queue_max_size(queue_max_size),
    _perror_cb(perror_cb),
    _periodic_background_proc(static_cast<int>(batch_timeout_ms), watchdog, "Async batcher thread", perror_cb)
  {}

  async_batcher::~async_batcher() {
    // Stop the background procedure the queue before exiting
    _periodic_background_proc.stop();
    if (_queue.size() > 0) {
      flush();
    }
  }
}