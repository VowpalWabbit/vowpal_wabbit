// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/queue.h"

#include <atomic>
#include <functional>
#include <future>
#include <limits>
#include <memory>
#include <queue>
#include <thread>

// thread pool based off of the simple pool implementation from ccia:
// https://github.com/anthonywilliams/ccia_code_samples

namespace VW
{
class threads_joiner
{
public:
  explicit threads_joiner(std::vector<std::thread>& threads) : _threads{threads} {}
  ~threads_joiner()
  {
    for (std::thread& thread : _threads)
    {
      if (thread.joinable()) { thread.join(); }
    }
  }

private:
  std::vector<std::thread>& _threads;
};

class thread_pool
{
public:
  // Initializes a thread pool with num_threads threads.
  explicit thread_pool(size_t num_threads)
      : _done{false}, _task_queue{std::numeric_limits<size_t>::max()}, _joiner{_threads}
  {
    try
    {
      for (size_t i = 0; i < num_threads; i++) { _threads.push_back(std::thread(&thread_pool::worker, this)); }
    }
    catch (...)
    {
      _done = true;
      _task_queue.set_done();
      throw;
    }
  }

  ~thread_pool()
  {
    _done = true;
    _task_queue.set_done();
  }

  // enqueues a func task in the work queue for worker threads to execute.
  template <typename F, typename... Args>
  auto submit(F&& func, Args&&... args) -> std::future<decltype(func(args...))>
  {
    std::function<decltype(func(args...))()> f = std::bind(std::forward<F>(func), std::forward<Args>(args)...);

    if (_threads.size() == 0)
    {
      std::packaged_task<decltype(func(args...))()> pt(f);
      pt();
      return pt.get_future();
    }

    // wrap into a shared pointer in order for the packaged task to be copiable
    auto task_ptr = std::make_shared<std::packaged_task<decltype(func(args...))()>>(f);
    std::function<void()> wrapper_func = [task_ptr]() { (*task_ptr)(); };

    _task_queue.push(std::move(wrapper_func));
    return task_ptr->get_future();
  }

  // returns the number of worker threads in the pool.
  size_t size() const { return _threads.size(); }

private:
  void worker()
  {
    while (!_done)
    {
      std::function<void()> task;
      if (!_task_queue.try_pop(task))
      { /*try pop returned false, the queue is done and it is empty*/
        return;
      }
      task();
    }
  }

  std::atomic_bool _done;
  VW::thread_safe_queue<std::function<void()>> _task_queue;
  std::vector<std::thread> _threads;
  threads_joiner _joiner;
};
}  // namespace VW