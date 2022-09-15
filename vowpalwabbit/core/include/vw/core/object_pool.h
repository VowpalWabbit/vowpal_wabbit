// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <cassert>
#include <queue>
#include <set>
#include <stack>

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
struct no_lock_object_pool
{
  void return_object(T* obj) { m_pool.push(std::unique_ptr<T>(obj)); }

  void return_object(std::unique_ptr<T> obj) { m_pool.push(std::move(obj)); }

  std::unique_ptr<T> get_object()
  {
    if (m_pool.empty()) { return std::unique_ptr<T>(new T); }

    auto obj = std::move(m_pool.top());
    m_pool.pop();
    return obj;
  }

private:
  std::stack<std::unique_ptr<T>> m_pool;
};

template <typename T>
struct moved_object_pool
{
  moved_object_pool() = default;

  void reclaim_object(T&& obj) { m_pool.push(std::move(obj)); }

  void acquire_object(T& dest)
  {
    if (m_pool.empty())
    {
      dest = T();
      return;
    }

    dest = std::move(m_pool.top());
    m_pool.pop();
  }

  bool empty() const { return m_pool.empty(); }

  size_t size() const { return m_pool.size(); }

private:
  std::stack<T> m_pool;
};

template <typename T>
struct object_pool
{
  void return_object(T* obj)
  {
    std::unique_lock<std::mutex> lock(m_lock);
    m_inner_pool.return_object(obj);
  }

  void return_object(std::unique_ptr<T> obj)
  {
    std::unique_lock<std::mutex> lock(m_lock);
    m_inner_pool.return_object(std::move(obj));
  }

  std::unique_ptr<T> get_object()
  {
    std::unique_lock<std::mutex> lock(m_lock);
    return m_inner_pool.get_object();
  }

private:
  mutable std::mutex m_lock;
  no_lock_object_pool<T> m_inner_pool;
};
}  // namespace VW
