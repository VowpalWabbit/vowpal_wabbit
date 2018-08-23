#pragma once

#include <queue>
#include <mutex>


namespace reinforcement_learning {

  //a moving concurrent queue with locks and mutex
  template <class T>
  class moving_queue {

    std::queue<T> _queue;
    std::mutex _mutex;

  public:

    void pop(T* item)
    {
      std::unique_lock<std::mutex> mlock(_mutex);
      if (!_queue.empty())
      {
        *item = std::move(_queue.front());
        _queue.pop();
      }
    }

    void push(T& item) {
      push(std::move(item));
    }

    void push(T&& item)
    {
      std::unique_lock<std::mutex> mlock(_mutex);
      _queue.push(std::move(item));
    }

    //approximate size
    size_t size()
    {
      std::unique_lock<std::mutex> mlock(_mutex);
      return _queue.size();
    }
  };
}

