#pragma once

#include <string>
#include <thread>
#include "moving_queue.h"
#include "api_status.h"
#include "../error_callback_fn.h"
#include "err_constants.h"
#include "utility/data_buffer.h"

namespace reinforcement_learning {
  class error_callback_fn;

  // This class takes uses a queue and a background thread to accumulate events, and send them by batch asynchronously.
  // A batch is shipped with TSender::send(data)
  template <typename TSender>
  class async_batcher
  {
  public:
    int init(api_status* status);

    int append(std::string&& evt, api_status* status = nullptr);
    int append(std::string& evt, api_status* status = nullptr);

    private:
    void timer(); //the timer triggers a queue flush (run in background)
    size_t fill_buffer(size_t remaining, std::string& buf_to_send);
    void flush(); //flush all batches

  public:
    async_batcher(TSender& pipe,
                  error_callback_fn* perror_cb = nullptr,
                  size_t send_high_water_mark = (1024 * 1024 * 4),
                  size_t batch_timeout_ms = 1000,
                  size_t queue_max_size = (8 * 1024));

    ~async_batcher();

  private:
    TSender& _sender;                     //somewhere to send the batch of data

    moving_queue<std::string> _queue;     //a queue to accumulate batch of events
    std::thread _background_thread;       //a background thread runs a timer that flushes the queue
    bool _thread_is_running;
    utility::data_buffer _buffer;           //re-used buffer to prevent re-allocation during sends
    size_t _send_high_water_mark;
    size_t _batch_timeout_ms;
    size_t _queue_max_size;
    error_callback_fn* _perror_cb;
  };

  template <typename TSender>
  int async_batcher<TSender>::init(api_status* status) {
    if ( !_thread_is_running ) {
      try {
        _thread_is_running = true;
        _background_thread = std::thread(&async_batcher::timer, this);
      }
      catch(const std::exception& e) {
        _thread_is_running = false;
        RETURN_ERROR_LS(status, background_thread_start) << " (logger)" << e.what();
      }
    }
    return error_code::success;
  }

  template <typename TSender>
  int async_batcher<TSender>::append(std::string&& evt, api_status* status) {
    if ( _queue.size() < _queue_max_size ) {
      _queue.push(std::move(evt));
      return error_code::success;
    }
    RETURN_ERROR_LS(status, background_queue_overflow) << "Queue size: " << _queue.size() << "Dropped event: " << evt ;
  }

  template <typename TSender>
  int async_batcher<TSender>::append(std::string& evt, api_status* status) {
    return append(std::move(evt), status);
  }

  template <typename TSender>
  void async_batcher<TSender>::timer()
  {
    while ( _thread_is_running ) {
      std::this_thread::sleep_for(std::chrono::milliseconds(_batch_timeout_ms));
      flush();
    }
  }

  template <typename TSender>
  size_t async_batcher<TSender>::fill_buffer(size_t remaining, std::string& buf_to_send)
  {
    // There is at least one element.  Pop uses move assignment
    // Copy can be avoided if send size is satisfied
    _queue.pop(&buf_to_send);
    auto filled_size = buf_to_send.size();
    --remaining;
    if ( remaining <= 0 || filled_size >= _send_high_water_mark ) {
      return remaining;
    }

    // Send size not satisfied.  Shift to larger buffer to satisfy send.
    // Copy is needed but reuse existing tmp buffer to avoid allocation
    _buffer.reset();
    _buffer << buf_to_send;

    while( remaining > 0 && filled_size < _send_high_water_mark) {
      _queue.pop(&buf_to_send);
      _buffer << "\n" << buf_to_send;
      --remaining;
      filled_size += buf_to_send.size();
    }
    buf_to_send = std::move(_buffer.str());
    return remaining;
  }

  template <typename TSender>
  void async_batcher<TSender>::flush() {
    const auto queue_size = _queue.size();
    if (queue_size == 0) return;
    auto remaining = queue_size;
    std::string buf_to_send;
    //handle batching
    while (remaining > 0) {
      remaining = fill_buffer(remaining, buf_to_send);
      api_status status;
      if ( _sender.send(buf_to_send, &status) != error_code::success )
        ERROR_CALLBACK(_perror_cb, status);
    }
  }

  template <typename TSender>
  async_batcher<TSender>::async_batcher(TSender& pipe, error_callback_fn* perror_cb, const size_t send_high_water_mark,
                                        const size_t batch_timeout_ms, const size_t queue_max_size)
              : _sender(pipe),
              _thread_is_running(false),
              _send_high_water_mark(send_high_water_mark),
              _batch_timeout_ms(batch_timeout_ms),
              _queue_max_size(queue_max_size) ,
              _perror_cb(perror_cb)
  {
  }

  template <typename TSender>
  async_batcher<TSender>::~async_batcher() {
    //stop the thread and flush the queue before exiting
    _thread_is_running = false;
    if (_background_thread.joinable())
      _background_thread.join();
    if (_queue.size() > 0)
      flush();
  }
}
