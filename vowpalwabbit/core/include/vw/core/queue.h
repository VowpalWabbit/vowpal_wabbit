// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <queue>

// Mutex and CV cannot be used in managed C++, tell the compiler that this is unmanaged even if included in a managed
// project.
#ifdef _M_CEE
#  pragma managed(push, off)
#  undef _M_CEE
#  include <condition_variable>
#  include <mutex>
#  define _M_CEE 001
#  pragma managed(pop)
#else
#  include <condition_variable>
#  include <mutex>
#endif

namespace VW
{
template <typename T>
class thread_safe_queue
{
public:
  thread_safe_queue(size_t max_size) : _max_size(max_size) {}

  bool try_pop(T& item)
  {
    std::unique_lock<std::mutex> lock(_mut);
    while (_object_queue.size() == 0 && !_done) { _is_not_empty.wait(lock); }

    if (_done && _object_queue.size() == 0) { return false; }

    item = std::move(_object_queue.front());
    _object_queue.pop();

    _is_not_full.notify_all();
    return true;
  }

  void push(T item)
  {
    std::unique_lock<std::mutex> lock(_mut);
    while (_object_queue.size() == _max_size) { _is_not_full.wait(lock); }
    _object_queue.push(std::move(item));

    _is_not_empty.notify_all();
  }

  void set_done()
  {
    {
      std::unique_lock<std::mutex> lock(_mut);
      _done = true;
    }
    _is_not_empty.notify_all();
    _is_not_full.notify_all();
  }

  size_t size() const
  {
    std::unique_lock<std::mutex> lock(_mut);
    return _object_queue.size();
  }

private:
  size_t _max_size;
  std::queue<T> _object_queue;
  mutable std::mutex _mut;

  volatile bool _done = false;

  std::condition_variable _is_not_full;
  std::condition_variable _is_not_empty;
};
}  // namespace VW
