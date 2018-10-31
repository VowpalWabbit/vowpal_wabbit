#pragma once

#include <string>

#include "event_queue.h"
#include "ranking_event.h"
#include "sender.h"
#include "api_status.h"
#include "../error_callback_fn.h"
#include "err_constants.h"
#include "utility/data_buffer.h"
#include "utility/periodic_background_proc.h"

namespace reinforcement_learning {
  //this enum sets the behavior of the queue managed by the async_batcher
  enum queue_mode_enum {
    DROP,//queue drops events if it is full (default)
    BLOCK//queue block if it is full
  };

  //convert a string to an enum value
  queue_mode_enum to_queue_mode_enum(const char* queue_mode);
};

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
    size_t fill_buffer(size_t remaining);
    void flush(); //flush all batches

  public:
    async_batcher(i_sender* sender,
                  utility::watchdog& watchdog,
                  error_callback_fn* perror_cb = nullptr,
                  size_t send_high_water_mark = (1024 * 1024 * 4),
                  size_t batch_timeout_ms = 1000,
                  size_t queue_max_capacity = (16 * 1024 * 1024),
                  queue_mode_enum queue_mode = DROP);
    ~async_batcher();

  private:
    std::unique_ptr<i_sender> _sender;

    event_queue<TEvent> _queue;       // A queue to accumulate batch of events.
    utility::data_buffer _buffer;           // Re-used buffer to prevent re-allocation during sends.
    size_t _send_high_water_mark;
    size_t _queue_max_capacity;
    error_callback_fn* _perror_cb;

    utility::periodic_background_proc<async_batcher> _periodic_background_proc;
    float _pass_prob;
    queue_mode_enum _queue_mode;
    std::condition_variable _cv;
    std::mutex _m;
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

    //block or drop events if the queue if full
    if (_queue.capacity() >= _queue_max_capacity) {
      if (BLOCK == _queue_mode) {
        std::unique_lock<std::mutex> lk(_m);
        _cv.wait(lk, [this] { return _queue.capacity() < _queue_max_capacity; });
      }
      else if (DROP == _queue_mode) {
        _queue.prune(_pass_prob);
      }
    }

    return error_code::success;
  }

  template<typename TEvent>
  int async_batcher<TEvent>::append(TEvent& evt, api_status* status) {
    return append(std::move(evt), status);
  }

  template<typename TEvent>
  int async_batcher<TEvent>::run_iteration(api_status* status) {
    flush();
    return error_code::success;
  }

  template<typename TEvent>
  size_t async_batcher<TEvent>::fill_buffer(size_t remaining)
  {
    TEvent evt;
    _buffer.reset();

    while (remaining > 0 && _buffer.size() < _send_high_water_mark) {
      _queue.pop(&evt);
      if (BLOCK == _queue_mode) {
        _cv.notify_one();
      }
      evt.serialize(_buffer);
      _buffer << "\n";
      --remaining;
    }
    _buffer.remove_last();

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
    // Handle batching
    while (remaining > 0) {
      remaining = fill_buffer(remaining);
      api_status status;
      if (_sender->send(_buffer.str(), &status) != error_code::success) {
        ERROR_CALLBACK(_perror_cb, status);
      }
    }
  }

  template<typename TEvent>
  async_batcher<TEvent>::async_batcher(i_sender* sender, utility::watchdog& watchdog, error_callback_fn* perror_cb, const size_t send_high_water_mark,
    const size_t batch_timeout_ms, const size_t queue_max_capacity, queue_mode_enum queue_mode)
    : _sender(sender),
    _send_high_water_mark(send_high_water_mark),
    _queue_max_capacity(queue_max_capacity),
    _perror_cb(perror_cb),
    _periodic_background_proc(static_cast<int>(batch_timeout_ms), watchdog, "Async batcher thread", perror_cb),
    _pass_prob(0.5),
    _queue_mode(queue_mode)
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
