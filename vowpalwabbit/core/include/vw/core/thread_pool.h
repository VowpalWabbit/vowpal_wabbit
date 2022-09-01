// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/memory.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

// thread pool based off of the simple pool implementation from ccia:
// https://github.com/anthonywilliams/ccia_code_samples

namespace VW
{
template <typename T>
class thread_safe_queue
{
public:
  thread_safe_queue() {}
  void push(T value)
  {
    std::lock_guard<std::mutex> lk(_mut);
    _queue.push(std::move(value));
  }

  bool try_pop(T& value)
  {
    std::lock_guard<std::mutex> lk(_mut);
    if (_queue.empty()) return false;
    value = std::move(_queue.front());
    _queue.pop();
    return true;
  }

  bool empty() const
  {
    std::lock_guard<std::mutex> lk(_mut);
    return _queue.empty();
  }

private:
  mutable std::mutex _mut;
  std::queue<T> _queue;
};

class threads_joiner
{
public:
  explicit threads_joiner(std::vector<std::thread>& threads) : _threads{threads} {}
  ~threads_joiner()
  {
    for (std::thread& thread : _threads)
    {
      if (thread.joinable()) thread.join();
    }
  }

private:
  std::vector<std::thread>& _threads;
};

class thread_pool
{
private:
  class function_wrapper
  {
    // function wrapper is a type erased move-only function storer needed in order to temporarily store the submitted
    // task
    // in a std::packaged_task which in turn is move-only
  public:
    template <typename F>
    function_wrapper(F&& f) : _impl{VW::make_unique<impl_type<F>>(std::forward<F>(f))}
    {
    }

    function_wrapper() = default;

    function_wrapper(function_wrapper&& other) : _impl{std::move(other._impl)} {}

    function_wrapper(const function_wrapper&) = delete;

    function_wrapper(function_wrapper&) = delete;

    function_wrapper& operator=(const function_wrapper&) = delete;

    void operator()() { _impl->call(); }

    function_wrapper& operator=(function_wrapper&& other)
    {
      _impl = std::move(other._impl);
      return *this;
    }

  private:
    struct impl_base
    {
      virtual void call() = 0;
      virtual ~impl_base() {}
    };

    template <typename F>
    struct impl_type : impl_base
    {
      F _f;
      impl_type(F&& f) : _f{std::forward<F>(f)} {}
      void call() { _f(); }
    };
    std::unique_ptr<impl_base> _impl;
  };

  void worker()
  {
    while (!_done)
    {
      function_wrapper task;
      {
        std::unique_lock<std::mutex> l(_mut);
        while (!_task_queue.try_pop(task) && !_done) { _cv.wait(l); }
      }
      if (_done) { break; }
      task();
    }
  }

  std::atomic_bool _done;
  std::condition_variable _cv;
  mutable std::mutex _mut;
  thread_safe_queue<function_wrapper> _task_queue;
  std::vector<std::thread> _threads;
  threads_joiner _joiner;

public:
  // Initializes a thread pool with num_threads threads.
  explicit thread_pool(size_t num_threads) : _done{false}, _joiner{_threads}
  {
    try
    {
      for (size_t i = 0; i < num_threads; i++) { _threads.push_back(std::thread(&thread_pool::worker, this)); }
    }
    catch (...)
    {
      _done = true;
      _cv.notify_all();
      throw;
    }
  }

  ~thread_pool()
  {
    _done = true;
    _cv.notify_all();
  }

  // Enqueues a func task in the work queue for worker threads to execute.
  template <typename F, typename... Args>
  std::future<typename std::result_of<F(Args...)>::type> submit(F&& func, Args&&... args)
  {
    using ResultType = typename std::result_of<F(Args...)>::type;
    std::packaged_task<ResultType()> task(std::bind(std::forward<F>(func), std::forward<Args>(args)...));
    std::future<ResultType> result(task.get_future());

    if (_threads.size() == 0)
    {
      task();
      return result;
    }

    {
      std::unique_lock<std::mutex> l(_mut);
      _task_queue.push(std::move(task));
    }

    _cv.notify_one();
    return result;
  }

  // Returns the number of worker threads in the pool.
  size_t size() const { return _threads.size(); }
};
}  // namespace VW