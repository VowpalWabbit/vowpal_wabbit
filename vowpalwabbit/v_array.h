// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#ifndef NOMINMAX
#  define NOMINMAX
#endif

#include <utility>
#include <cstdlib>
#include "future_compat.h"

#ifndef VW_NOEXCEPT
#  include "vw_exception.h"
#endif

#include "memory.h"

template <class T>
struct v_array
{
private:
  static constexpr size_t ERASE_POINT = ~((1u << 10u) - 1u);

  void delete_v_array()
  {
    if (_begin != nullptr)
    {
      for (iterator item = _begin; item != _end; ++item) { item->~T(); }
      free(_begin);
    }
    _begin = nullptr;
    _end = nullptr;
    end_array = nullptr;
    erase_count = 0;
  }

public:
  // private:
  T* _begin;
  T* _end;

public:
  T* end_array;
  size_t erase_count;

  using value_type = T;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using iterator = value_type*;
  using const_iterator = const value_type*;

  // enable C++ 11 for loops
  inline iterator& begin() { return _begin; }
  inline iterator& end() { return _end; }

  inline const_iterator begin() const { return _begin; }
  inline const_iterator end() const { return _end; }

  inline const_iterator cbegin() const { return _begin; }
  inline const_iterator cend() const { return _end; }

  v_array() noexcept : _begin(nullptr), _end(nullptr), end_array(nullptr), erase_count(0) {}
  ~v_array() { delete_v_array(); }

  v_array(v_array<T>&& other) noexcept
  {
    erase_count = 0;
    _begin = nullptr;
    _end = nullptr;
    end_array = nullptr;

    std::swap(_begin, other._begin);
    std::swap(_end, other._end);
    std::swap(end_array, other.end_array);
    std::swap(erase_count, other.erase_count);
  }

  v_array<T>& operator=(v_array<T>&& other) noexcept
  {
    std::swap(_begin, other._begin);
    std::swap(_end, other._end);
    std::swap(end_array, other.end_array);
    std::swap(erase_count, other.erase_count);
    return *this;
  }

  v_array(const v_array<T>& other)
  {
    _begin = nullptr;
    _end = nullptr;
    end_array = nullptr;
    erase_count = 0;

    // TODO this should use the other version when T is trivially copyable and this otherwise.
    copy_array_no_memcpy(*this, other);
  }

  v_array<T>& operator=(const v_array<T>& other)
  {
    if (this == &other) return *this;

    delete_v_array();
    // TODO this should use the other version when T is trivially copyable and this otherwise.
    copy_array_no_memcpy(*this, other);
    return *this;
  }

  inline T& back() { return *(_end - 1); }
  inline const T& back() const { return *(_end - 1); }

  inline void pop_back()
  {
    // Check if the v_array is empty.
    assert(_begin != _end);
    (--_end)->~T();
  }

  VW_DEPRECATED("v_array::pop() is deprecated. Use pop_back()")
  T pop()
  {
    T ret = back();
    pop_back();
    return ret;
  }

  VW_DEPRECATED("v_array::last() is deprecated. Use back()")
  T last() const { return *(_end - 1); }

  bool empty() const { return _begin == _end; }
  void decr() { _end--; }
  void incr()
  {
    if (_end == end_array) resize(2 * capacity() + 3);
    _end++;
  }
  T& operator[](size_t i) const { return _begin[i]; }
  inline size_t size() const { return _end - _begin; }
  inline size_t capacity() const { return end_array - _begin; }

  void resize(size_t length)
  {
    if (capacity() != length)
    {
      size_t old_len = _end - _begin;
      T* temp = (T*)realloc(_begin, sizeof(T) * length);
      if ((temp == nullptr) && ((sizeof(T) * length) > 0))
      { THROW_OR_RETURN("realloc of " << length << " failed in resize().  out of memory?"); }
      else
        _begin = temp;
      if (old_len < length && _begin + old_len != nullptr) memset(_begin + old_len, 0, (length - old_len) * sizeof(T));
      _end = _begin + old_len;
      end_array = _begin + length;
    }
  }

  void clear()
  {
    if (++erase_count & ERASE_POINT)
    {
      resize(_end - _begin);
      erase_count = 0;
    }
    for (T* item = _begin; item != _end; ++item) item->~T();
    _end = _begin;
  }
  void delete_v()
  {
    if (_begin != nullptr)
    {
      for (T* item = _begin; item != _end; ++item) item->~T();
      free(_begin);
    }
    _begin = _end = end_array = nullptr;
  }
  void push_back(const T& new_ele)
  {
    if (_end == end_array) resize(2 * capacity() + 3);
    new (_end++) T(new_ele);
  }

  void push_back_unchecked(const T& new_ele) { new (_end++) T(new_ele); }

  template <class... Args>
  void emplace_back(Args&&... args)
  {
    if (_end == end_array) resize(2 * capacity() + 3);
    new (_end++) T(std::forward<Args>(args)...);
  }

  size_t find_sorted(const T& ele) const  // index of the smallest element >= ele, return true if element is in the
                                          // array
  {
    size_t size = _end - _begin;
    size_t a = 0;
    size_t b = size;
    size_t i = (a + b) / 2;

    while (b - a > 1)
    {
      if (_begin[i] < ele)  // if a = 0, size = 1, if in while we have b - a >= 1 the loop is infinite
        a = i;
      else if (_begin[i] > ele)
        b = i;
      else
        return i;

      i = (a + b) / 2;
    }

    if ((size == 0) || (_begin[a] > ele) || (_begin[a] == ele))  // pusta tablica, nie wchodzi w while
      return a;
    else  // size = 1, ele = 1, _begin[0] = 0
      return b;
  }
  size_t unique_add_sorted(const T& new_ele)
  {
    size_t index = 0;
    size_t size = _end - _begin;
    size_t to_move;

    if (!contain_sorted(new_ele, index))
    {
      if (_end == end_array) resize(2 * capacity() + 3);

      to_move = size - index;

      if (to_move > 0)
        memmove(_begin + index + 1, _begin + index,
            to_move * sizeof(T));  // kopiuje to_move*.. bytow z _begin+index do _begin+index+1

      _begin[index] = new_ele;

      _end++;
    }

    return index;
  }
  bool contain_sorted(const T& ele, size_t& index)
  {
    index = find_sorted(ele);

    if (index == this->size()) return false;

    if (_begin[index] == ele) return true;

    return false;
  }
};

template <class T>
inline v_array<T> v_init()
{
  return v_array<T>();
}

template <class T>
void copy_array(v_array<T>& dst, const v_array<T>& src)
{
  dst.clear();
  push_many(dst, src._begin, src.size());
}

// use to copy arrays of types with non-trivial copy constructors, such as shared_ptr
template <class T>
void copy_array_no_memcpy(v_array<T>& dst, const v_array<T>& src)
{
  dst.clear();
  for (T* item = src._begin; item != src._end; ++item) dst.push_back(*item);
}

template <class T>
void copy_array(v_array<T>& dst, const v_array<T>& src, T (*copy_item)(T&))
{
  dst.clear();
  for (T* item = src._begin; item != src._end; ++item) dst.push_back(copy_item(*item));
}

template <class T>
void push_many(v_array<T>& v, const T* src, size_t num)
{
  if (v._end + num >= v.end_array)
    v.resize(std::max(2 * (size_t)(v.end_array - v._begin) + 3, v._end - v._begin + num));
#ifdef _WIN32
  memcpy_s(v._end, (v.end_array - v._end) * sizeof(T), src, num * sizeof(T));
#else
  memcpy(v._end, src, num * sizeof(T));
#endif
  v._end += num;
}

template <class T>
void calloc_reserve(v_array<T>& v, size_t length)
{
  v._begin = calloc_or_throw<T>(length);
  v._end = v._begin;
  v.end_array = v._begin + length;
}

template <class T>
bool v_array_contains(v_array<T>& A, T x)
{
  for (T* e = A._begin; e != A._end; ++e)
    if (*e == x) return true;
  return false;
}
