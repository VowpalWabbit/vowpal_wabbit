#pragma once

#include <set>
#include <queue>

namespace VW
{
template <typename T>
struct object_pool
{
  virtual void return_object(T*) = 0;
  virtual T* get_object() = 0;
  virtual bool empty() const = 0;
  virtual size_t available() const = 0;
  virtual size_t size() const = 0;
  virtual bool is_from_pool(T*) const = 0;

  virtual ~object_pool() {}
};

// Not thread safe
template <typename T, typename TFactory>
struct unbounded_object_pool : object_pool<T>
{
  unbounded_object_pool() = default;
  unbounded_object_pool(size_t initial_size)
  {
    for (size_t i = 0; i < initial_size; i++)
    {
      auto obj = m_factory();
      m_pool_objects.insert(obj);
      m_pool.push(std::unique_ptr<T>(obj));
    }
  }

  void return_object(T* obj) override
  {
    assert(is_from_pool(obj));
    m_pool.push(std::unique_ptr<T>(obj));
  }

  T* get_object() override
  {
    T* obj;
    if (m_pool.empty())
    {
      obj = m_factory();
      m_pool_objects.insert(obj);
    }
    else
    {
      obj = m_pool.front().release();
      m_pool.pop();
    }

    return obj;
  }

  bool empty() const override { return m_pool.empty(); }

  size_t available() const override { return m_pool.size(); }

  size_t size() const override { return m_pool_objects.size(); }

  bool is_from_pool(T* obj) const override { return m_pool_objects.count(obj) != 0; }

 private:
  std::queue<std::unique_ptr<T>> m_pool;
  std::set<T*> m_pool_objects;
  TFactory m_factory;
};
}  // namespace VW
