#pragma once

#include <string>
#include <thread>
#include <sstream>

#include "moving_queue.h"
#include "api_status.h"
#include "../error_callback_fn.h"

namespace reinforcement_learning {
  class error_callback_fn;

	// This class takes uses a queue and a background thread to accumulate events, and send them by batch asynchronously.
	// A batch is shipped with TSender::send(data)
	template <typename TSender>
	class async_batcher {

	public:

    int append(std::string&& evt, api_status* status = nullptr) {
      if ( _queue.size() < _queue_max_size ) {
        _queue.push(std::move(evt));
        return error_code::success;
      }

      //report errors
      std::ostringstream os;
      os << "Dropped event: " << evt;
      api_status::try_update(status, error_code::background_queue_overflow, os.str().c_str());

      return error_code::background_queue_overflow;
    }

		int append(std::string& evt, api_status* status = nullptr)
		{
      return append(std::move(evt), status);
		}

	private:
		void timer()//the timer triggers a queue flush (run in background)
		{
			while (_thread_is_running)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(_batch_timeout_ms));
				flush();
			}
		}

		void flush()//flush all batches
		{
		  const auto queue_size = _queue.size();
			if (queue_size == 0) return;

			//handle batching
			std::string batch, next;
			_queue.pop(&batch); //there is at least one element

			for (size_t i = 1; i < queue_size; ++i)
			{
				_queue.pop(&next);

				size_t batch_size = batch.length() + next.length();
				if (batch_size > _batch_max_size)
				{
          api_status status;
					//the batch is about to reach the max size: send it
					if(_sender.send(batch,&status) != error_code::success)
					{
            if (_perror_cb) _perror_cb->report_error(status);
					}
          
					batch = next;
				}
				else
					batch += "\n" + next;
			}

			//send remaining events
			if (batch.size() > 0)
			{
        api_status status;
        //the batch is about to reach the max size: send it
        if (_sender.send(batch, &status) != error_code::success)
        {
          if (_perror_cb) _perror_cb->report_error(status);
        }
			}
		}

	public:
		async_batcher(TSender& pipe, error_callback_fn* perror_cb = nullptr,
                  size_t batch_max_size = (256 * 1024 - 1), size_t batch_timeout_ms = 1000, size_t queue_max_size = (8 * 1024))
			: _sender(pipe),
	    _batch_max_size(batch_max_size),
			_batch_timeout_ms(batch_timeout_ms),
			_queue_max_size(queue_max_size),
      _perror_cb(perror_cb)
		{
			_thread_is_running = true;
			_background_thread = std::thread(&async_batcher::timer, this);
		}

		~async_batcher()
		{
			//stop the thread and flush the queue before exiting
			_thread_is_running = false;
			if (_background_thread.joinable())
				_background_thread.join();
			if (_queue.size() > 0)
				flush();
		}


	private:
		TSender& _sender;                     //somewhere to send the batch of data

		moving_queue<std::string> _queue;     //a queue to accumulate batch of events
		std::thread _background_thread;       //a background thread runs a timer that flushes the queue
		bool _thread_is_running;

		size_t _batch_max_size;
		size_t _batch_timeout_ms;
		size_t _queue_max_size;
    error_callback_fn* _perror_cb;
	};
}
