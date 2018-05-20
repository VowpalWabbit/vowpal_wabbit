#pragma once

#include <queue>
#include <mutex>


namespace reinforcement_learning {

	//a concurrent queue with locks and mutex
	template <class T>
	class concurrent_queue {

		std::queue<T> _queue;
		std::mutex _mutex;

	public:

		void pop(T* item)
		{
			std::unique_lock<std::mutex> mlock(_mutex);
			if (!_queue.empty())
			{
				*item = _queue.front();
				_queue.pop();
			}
		}

		void push(const T& item)
		{
			std::unique_lock<std::mutex> mlock(_mutex);
			_queue.push(item);
		}

		//approximate size
		size_t size() const
		{
			return _queue.size();
		}
	};
}

