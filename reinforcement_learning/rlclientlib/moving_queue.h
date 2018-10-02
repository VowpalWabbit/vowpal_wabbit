#pragma once

#include <cstddef>
#include <queue>

namespace reinforcement_learning {

  //a moving non-concurrent queue
  template <class T>
  class moving_queue {
    using queue_t = std::queue<T>;

    queue_t _queue;

  public:
    void pop(T* item)
    {
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
      _queue.push(std::forward<T>(item));
    }

    //approximate size
    size_t size()
    {
      return _queue.size();
    }
  };
}

