#pragma once

#include <string>

#include "moving_queue.h"
#include "message.h"
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
  class async_batcher {
  public:
    int init(api_status* status);

    int append(message&& evt, api_status* status = nullptr);
    int append(message& evt, api_status* status = nullptr);

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

    moving_queue<message> _queue;       // A queue to accumulate batch of events.
    utility::data_buffer _buffer;           // Re-used buffer to prevent re-allocation during sends.
    size_t _send_high_water_mark;
    size_t _queue_max_size;
    error_callback_fn* _perror_cb;

    utility::periodic_background_proc<async_batcher> _periodic_background_proc;
    float _pass_drop_prob;
  };
}
