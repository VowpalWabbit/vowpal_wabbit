#pragma once

#include <set>
#include <queue>

// Mutex and CV cannot be used in managed C++, tell the compiler that this is unmanaged even if included in a managed
// project.
#ifdef _M_CEE
#pragma managed(push, off)
#undef _M_CEE
#include <mutex>
#include <condition_variable>
#define _M_CEE 001
#pragma managed(pop)
#else
#include <mutex>
#include <condition_variable>
#endif

namespace VW
{
// Not thread safe
template <typename T, typename TInitializer>
struct object_pool
{
  object_pool() = default;
  object_pool(size_t initial_chunk_size, TInitializer initializer = {}, size_t chunk_size = 8)
      : m_initializer(initializer), m_initial_chunk_size(initial_chunk_size), m_chunk_size(chunk_size)
  {
    new_chunk(initial_chunk_size);
  }

  void return_object(T* obj)
  {
    std::unique_lock<std::mutex> lock(m_lock);
    assert(is_from_pool(obj));
    m_pool.push(obj);
  }

  T* get_object()
  {
    std::unique_lock<std::mutex> lock(m_lock);
    if (m_pool.empty())
    {
      new_chunk(m_chunk_size);
    }

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

  bool is_from_pool(T* obj) const
  {
    for (auto& bound : m_chunk_bounds)
    {
      if (obj >= bound.first && obj <= bound.second)
      {
        return true;
      }
    }

    return false;
  }

 private:
  void new_chunk(size_t size)
  {
    if (size == 0)
    {
      return;
    }

    m_chunks.push_back(std::unique_ptr<T[]>(new T[size]));
    auto& chunk = m_chunks.back();
    m_chunk_bounds.push_back({&chunk[0], &chunk[size - 1]});

    for (size_t i = 0; i < size; i++)
    {
      memset(&chunk[i], 0, sizeof(T));
      new (&chunk[i]) T{};
      m_pool.push(m_initializer(&chunk[i]));
    }
  }

  std::mutex m_lock;
  std::queue<T*> m_pool;
  std::vector<std::pair<T*, T*>> m_chunk_bounds;
  std::vector<std::unique_ptr<T[]>> m_chunks;
  TInitializer m_initializer;
  size_t m_initial_chunk_size = 0;
  size_t m_chunk_size = 8;
};
}  // namespace VW
