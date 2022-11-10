// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <cassert>
#include <queue>
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
class default_cleanup
{
public:
  void operator()(T*) {}
};

template <typename T>
class default_initializer
{
public:
  T* operator()(T* obj) { return obj; }
};

template <typename T, typename TInitializer = default_initializer<T>, typename TCleanup = default_cleanup<T>>
class no_lock_object_pool
{
public:
  no_lock_object_pool() = default;
  no_lock_object_pool(size_t initial_chunk_size, TInitializer initializer = {}, size_t chunk_size = 8)
      : _initializer(initializer), _initial_chunk_size(initial_chunk_size), _chunk_size(chunk_size)
  {
    new_chunk(initial_chunk_size);
  }

  ~no_lock_object_pool()
  {
    assert(_pool.size() == size());
    while (!_pool.empty())
    {
      auto front = _pool.front();
      _cleanup(front);
      _pool.pop();
    }
  }

  void return_object(T* obj)
  {
    assert(is_from_pool(obj));
    _pool.push(obj);
  }

  T* get_object()
  {
    if (_pool.empty()) { new_chunk(_chunk_size); }

    auto obj = _pool.front();
    _pool.pop();
    return obj;
  }

  bool empty() const { return _pool.empty(); }

  size_t size() const
  {
    size_t size = 0;
    auto num_chunks = _chunk_bounds.size();

    if (_chunk_bounds.size() > 0 && _initial_chunk_size > 0)
    {
      size += _initial_chunk_size;
      num_chunks--;
    }

    size += num_chunks * _chunk_size;
    return size;
  }

  bool is_from_pool(const T* obj) const
  {
    for (auto& bound : _chunk_bounds)
    {
      if (obj >= bound.first && obj <= bound.second) { return true; }
    }

    return false;
  }

private:
  void new_chunk(size_t size)
  {
    if (size == 0) { return; }

    _chunks.push_back(std::unique_ptr<T[]>(new T[size]));
    auto& chunk = _chunks.back();
    _chunk_bounds.push_back({&chunk[0], &chunk[size - 1]});

    for (size_t i = 0; i < size; i++) { _pool.push(_initializer(&chunk[i])); }
  }

  TInitializer _initializer;
  TCleanup _cleanup;
  size_t _initial_chunk_size = 0;
  size_t _chunk_size = 8;

  std::vector<std::unique_ptr<T[]>> _chunks;
  std::vector<std::pair<T*, T*>> _chunk_bounds;
  std::queue<T*> _pool;
};

template <typename T>
class moved_object_pool
{
public:
  moved_object_pool() = default;

  void reclaim_object(T&& obj) { _pool.push(std::move(obj)); }

  void acquire_object(T& dest)
  {
    if (_pool.empty())
    {
      dest = T();
      return;
    }

    dest = std::move(_pool.top());
    _pool.pop();
  }

  bool empty() const { return _pool.empty(); }

  size_t size() const { return _pool.size(); }

private:
  std::stack<T> _pool;
};

template <typename T, typename TInitializer = default_initializer<T>, typename TCleanup = default_cleanup<T>>
class object_pool
{
public:
  object_pool() = default;
  object_pool(size_t initial_chunk_size, TInitializer initializer = {}, size_t chunk_size = 8)
      : _inner_pool(initial_chunk_size, initializer, chunk_size)
  {
  }

  void return_object(T* obj)
  {
    std::unique_lock<std::mutex> lock(_lock);
    _inner_pool.return_object(obj);
  }

  T* get_object()
  {
    std::unique_lock<std::mutex> lock(_lock);
    return _inner_pool.get_object();
  }

  bool empty() const
  {
    std::unique_lock<std::mutex> lock(_lock);
    return _inner_pool.empty();
  }

  size_t size() const
  {
    std::unique_lock<std::mutex> lock(_lock);
    return _inner_pool.size();
  }

  bool is_from_pool(const T* obj) const
  {
    std::unique_lock<std::mutex> lock(_lock);
    return _inner_pool.is_from_pool(obj);
  }

private:
  mutable std::mutex _lock;
  no_lock_object_pool<T, TInitializer, TCleanup> _inner_pool;
};
}  // namespace VW
