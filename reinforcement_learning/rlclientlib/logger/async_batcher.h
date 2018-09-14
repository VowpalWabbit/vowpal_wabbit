#pragma once

#include <string>

#include "moving_queue.h"
#include "ranking_event.h"
#include "sender.h"
#include "api_status.h"
#include "../error_callback_fn.h"
#include "err_constants.h"
#include "utility/data_buffer.h"
#include "utility/periodic_background_proc.h"

namespace reinforcement_learning {
  class error_callback_fn;

  // This class takes uses a queue and a background thread to accumulate events, and send them by batch asynchronously.
  // A batch is shipped with TSender::send(data)
  template<typename TEvent>
  class async_batcher {
  public:
    int init(api_status* status);

    int append(TEvent&& evt, api_status* status = nullptr);
    int append(TEvent& evt, api_status* status = nullptr);

    int run_iteration(api_status* status);

  private:
    size_t fill_buffer(size_t remaining, std::string& buf_to_send);
    void prune_if_needed();
    void flush(); //flush all batches

  public:
    async_batcher(i_sender* sender,
                  utility::watchdog& watchdog,
                  error_callback_fn* perror_cb = nullptr,
                  size_t send_high_water_mark = (1024 * 1024 * 4),
                  size_t batch_timeout_ms = 1000,
                  size_t queue_max_size = (8 * 1024));

    ~async_batcher();

  private:
    std::unique_ptr<i_sender> _sender;

    moving_queue<TEvent> _queue;       // A queue to accumulate batch of events.
    utility::data_buffer _buffer;           // Re-used buffer to prevent re-allocation during sends.
    size_t _send_high_water_mark;
    size_t _queue_max_size;
    error_callback_fn* _perror_cb;

    utility::periodic_background_proc<async_batcher> _periodic_background_proc;
    float _pass_drop_prob;
  };

  template<typename TEvent>
  int async_batcher<TEvent>::init(api_status* status) {
    RETURN_IF_FAIL(_sender->init(status));
    RETURN_IF_FAIL(_periodic_background_proc.init(this, status));
    return error_code::success;
  }

  template<typename TEvent>
  int async_batcher<TEvent>::append(TEvent&& evt, api_status* status) {
    _queue.push(std::move(evt));
    return error_code::success;
  }

  template<typename TEvent>
  int async_batcher<TEvent>::append(TEvent& evt, api_status* status) {
    return append(std::move(evt), status);
  }

  template<typename TEvent>
  void async_batcher<TEvent>::prune_if_needed() {
    if (_queue.size() > _queue_max_size) {
      _queue.prune(_pass_drop_prob);
    }
  }

  template<typename TEvent>
  int async_batcher<TEvent>::run_iteration(api_status* status) {
    prune_if_needed();
    flush();
    return error_code::success;
  }

  template<typename TEvent>
  size_t async_batcher<TEvent>::fill_buffer(size_t remaining, std::string& buf_to_send)
  {
    // There is at least one element.  Pop uses move assignment
    // Copy can be avoided if send size is satisfied
    TEvent buf;
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

  template<typename TEvent>
  void async_batcher<TEvent>::flush() {
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

  template<typename TEvent>
  async_batcher<TEvent>::async_batcher(i_sender* sender, utility::watchdog& watchdog, error_callback_fn* perror_cb, const size_t send_high_water_mark,
    const size_t batch_timeout_ms, const size_t queue_max_size)
    : _sender(sender),
    _send_high_water_mark(send_high_water_mark),
    _queue_max_size(queue_max_size),
    _perror_cb(perror_cb),
    _periodic_background_proc(static_cast<int>(batch_timeout_ms), watchdog, "Async batcher thread", perror_cb),
    _pass_drop_prob(0.5)
  {}

  template<typename TEvent>
  async_batcher<TEvent>::~async_batcher() {
    // Stop the background procedure the queue before exiting
    _periodic_background_proc.stop();
    if (_queue.size() > 0) {
      flush();
    }
  }
}
