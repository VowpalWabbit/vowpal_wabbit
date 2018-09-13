#include "async_batcher.h"

namespace reinforcement_learning {
  int async_batcher::init(api_status* status) {
    RETURN_IF_FAIL(_sender->init(status));
    RETURN_IF_FAIL(_periodic_background_proc.init(this, status));
    return error_code::success;
  }

  int async_batcher::append(message&& evt, api_status* status) {
    _queue.push(std::move(evt));
    return error_code::success;
  }

  int async_batcher::append(message& evt, api_status* status) {
    return append(std::move(evt), status);
  }

  void async_batcher::prune_if_needed() {
    if (_queue.size() > _queue_max_size) {
      _queue.prune(_pass_drop_prob);
    }
  }

  int async_batcher::run_iteration(api_status* status) {
    prune_if_needed();
    flush();
    return error_code::success;
  }

  size_t async_batcher::fill_buffer(size_t remaining, std::string& buf_to_send)
  {
    // There is at least one element.  Pop uses move assignment
    // Copy can be avoided if send size is satisfied
    message buf;
    _queue.pop(&buf);
    buf_to_send = buf.str();
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
      _queue.pop(&buf);
      _buffer << "\n" << buf.str();
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
      if (_sender->send(buf_to_send, &status) != error_code::success) {
        ERROR_CALLBACK(_perror_cb, status);
      }
    }
  }

  async_batcher::async_batcher(i_sender* sender, utility::watchdog& watchdog, error_callback_fn* perror_cb, const size_t send_high_water_mark,
    const size_t batch_timeout_ms, const size_t queue_max_size)
    : _sender(sender),
    _send_high_water_mark(send_high_water_mark),
    _queue_max_size(queue_max_size),
    _perror_cb(perror_cb),
    _periodic_background_proc(static_cast<int>(batch_timeout_ms), watchdog, "Async batcher thread", perror_cb),
    _pass_drop_prob(0.5)
  {}

  async_batcher::~async_batcher() {
    // Stop the background procedure the queue before exiting
    _periodic_background_proc.stop();
    if (_queue.size() > 0) {
      flush();
    }
  }
}
