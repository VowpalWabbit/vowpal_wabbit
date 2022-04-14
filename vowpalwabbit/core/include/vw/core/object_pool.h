// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

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
struct default_cleanup
{
  void operator()(T*) {}
};

template <typename T>
struct default_initializer
{
  T* operator()(T* obj) { return obj; }
};

template <typename T, typename TInitializer = default_initializer<T>, typename TCleanup = default_cleanup<T>>
struct no_lock_object_pool
{
  no_lock_object_pool() = default;
  no_lock_object_pool(size_t initial_chunk_size, TInitializer initializer = {}, size_t chunk_size = 8)
      : m_initializer(initializer), m_initial_chunk_size(initial_chunk_size), m_chunk_size(chunk_size)
  {
    new_chunk(initial_chunk_size);
  }

  ~no_lock_object_pool()
  {
    assert(m_pool.size() == size());
    while (!m_pool.empty())
    {
      auto front = m_pool.front();
      m_cleanup(front);
      m_pool.pop();
    }
  }

  void return_object(T* obj)
  {
    assert(is_from_pool(obj));
    m_pool.push(obj);
  }

  T* get_object()
  {
    if (m_pool.empty()) { new_chunk(m_chunk_size); }

    auto obj = m_pool.front();
    m_pool.pop();
    return obj;
  }

  bool empty() const { return m_pool.empty(); }

  size_t size() const
  {
    size_t size = 0;
    auto num_chunks = m_chunk_bounds.size();

    if (m_chunk_bounds.size() > 0 && m_initial_chunk_size > 0)
    {
      size += m_initial_chunk_size;
      num_chunks--;
    }

    size += num_chunks * m_chunk_size;
    return size;
  }

  bool is_from_pool(const T* obj) const
  {
    for (auto& bound : m_chunk_bounds)
    {
      if (obj >= bound.first && obj <= bound.second) { return true; }
    }

    return false;
  }

private:
  void new_chunk(size_t size)
  {
    if (size == 0) { return; }

    m_chunks.push_back(std::unique_ptr<T[]>(new T[size]));
    auto& chunk = m_chunks.back();
    m_chunk_bounds.push_back({&chunk[0], &chunk[size - 1]});

    for (size_t i = 0; i < size; i++) { m_pool.push(m_initializer(&chunk[i])); }
  }

  TInitializer m_initializer;
  TCleanup m_cleanup;
  size_t m_initial_chunk_size = 0;
  size_t m_chunk_size = 8;

  std::vector<std::unique_ptr<T[]>> m_chunks;
  std::vector<std::pair<T*, T*>> m_chunk_bounds;
  std::queue<T*> m_pool;
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

template <typename T, typename TInitializer = default_initializer<T>, typename TCleanup = default_cleanup<T>>
struct object_pool
{
  object_pool() = default;
  object_pool(size_t initial_chunk_size, TInitializer initializer = {}, size_t chunk_size = 8)
      : inner_pool(initial_chunk_size, initializer, chunk_size)
  {
  }

  void return_object(T* obj)
  {
    std::unique_lock<std::mutex> lock(m_lock);
    inner_pool.return_object(obj);
  }

  T* get_object()
  {
    std::unique_lock<std::mutex> lock(m_lock);
    return inner_pool.get_object();
  }

  bool empty() const
  {
    std::unique_lock<std::mutex> lock(m_lock);
    return inner_pool.empty();
  }

  size_t size() const
  {
    std::unique_lock<std::mutex> lock(m_lock);
    return inner_pool.size();
  }

  bool is_from_pool(const T* obj) const
  {
    std::unique_lock<std::mutex> lock(m_lock);
    return inner_pool.is_from_pool(obj);
  }

private:
  mutable std::mutex m_lock;
  no_lock_object_pool<T, TInitializer, TCleanup> inner_pool;
};
}  // namespace VW
