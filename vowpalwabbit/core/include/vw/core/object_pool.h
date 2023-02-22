// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/future_compat.h"

#include <cassert>
#include <queue>
#include <stack>
#include <unordered_set>

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
namespace details
{
struct null_mutex
{
  void lock() {}
  void unlock() {}
  bool try_lock() { return true; }
};

template <typename T>
class default_factory
{
public:
  T* operator()() { return new T; }
};

template <typename T, typename TMutex = null_mutex, typename TFactory = default_factory<T>>
class object_pool_impl
{
public:
  object_pool_impl() = default;
  object_pool_impl(size_t initial_size, TFactory factory = {}) : _factory(factory)
  {
    for (size_t i = 0; i < initial_size; ++i) { _pool.push(allocate_new()); }
  }

  ~object_pool_impl() { assert(_pool.size() == size()); }

  void return_object(T* obj)
  {
    std::unique_lock<TMutex> lock(_lock);
    _pool.push(std::unique_ptr<T>(obj));
  }

  void return_object(std::unique_ptr<T> obj)
  {
    std::unique_lock<TMutex> lock(_lock);
    _pool.push(std::move(obj));
  }

  std::unique_ptr<T> get_object()
  {
    std::unique_lock<TMutex> lock(_lock);
    if (_pool.empty()) { return allocate_new(); }

    auto obj = std::move(_pool.front());
    _pool.pop();
    return obj;
  }

  bool empty() const
  {
    std::unique_lock<TMutex> lock(_lock);
    return _pool.empty();
  }

  size_t size() const
  {
    std::unique_lock<TMutex> lock(_lock);
    return _pool.size();
  }

  VW_DEPRECATED("Pools will no longer be able to check if an object is from the pool in VW 10.")
  bool is_from_pool(const T* obj) const
  {
    std::unique_lock<TMutex> lock(_lock);
    return no_lock_is_from_pool(obj);
  }

private:
  bool no_lock_is_from_pool(const T* obj) const { return _allocated_by_pool.find(obj) != _allocated_by_pool.end(); }

  std::unique_ptr<T> allocate_new()
  {
    auto* obj = _factory();
    _allocated_by_pool.insert(obj);
    return std::unique_ptr<T>(obj);
  }

  mutable TMutex _lock;
  TFactory _factory;
  std::unordered_set<const T*> _allocated_by_pool;
  std::queue<std::unique_ptr<T>> _pool;
};

}  // namespace details

template <typename T, typename TInitializer = details::default_factory<T>>
using no_lock_object_pool = details::object_pool_impl<T, details::null_mutex, TInitializer>;

template <typename T, typename TInitializer = details::default_factory<T>>
using object_pool = details::object_pool_impl<T, std::mutex, TInitializer>;

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
}  // namespace VW
