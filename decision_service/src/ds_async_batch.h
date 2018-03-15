#pragma once

#include "ds_configuration.h"
#include "ds_concurrent_queue.h"

#include <string>

namespace decision_service {

	template <typename TPipe>
	class async_batch {

	public:
		void append(const std::string& evt)
		{
			if (_queue.size() < _config.queue_max_size())
				_queue.push(evt);
			//TODO REPORT ERRORS
		}

		async_batch(const configuration& config, const std::string& url, const std::string& name)
			: _config(config),
			_pipe(config, url, name),
			_background_thread(&async_batch::timer, this),
			_timer_enabled(true)
		{}

		~async_batch()
		{
			//stop the thread and flush the queue before exiting
			_timer_enabled = false;
			_background_thread.join();
			flush();
		}


	private:
		void timer()//trigger a queue flush (run in background)
		{
			_timer_enabled = true;
			while (_timer_enabled)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(_config.batch_timeout_ms()));
				flush();
			}
		}

		void flush()//flush all batches
		{
			size_t queue_size = _queue.size();
			if (queue_size == 0) return;

			//handle batching
			std::string batch, next;
			_queue.pop(&batch);//there is at least one element

			for (size_t i = 1; i < queue_size; ++i)
			{
				_queue.pop(&next);

				size_t batch_size = batch.length() + next.length();
				if (batch_size > _config.batch_max_size())//TODO check null ref?
				{
					//the batch is about to reach the max size: send it
					_pipe.send(batch);
					batch = next;
				}
				else
					batch += "\n" + next;
			}

			//send remaining events
			if (batch.size()>0)
				_pipe.send(batch);
		}


	private:
		configuration _config;
		TPipe _pipe;
		std::thread _background_thread;      //a timer runs in a background thread
		bool _timer_enabled;                 //the timer triggers a task that flushes the queue
		concurrent_queue<std::string> _queue;//a queue to store events until they are sent
	};
}