// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#ifndef NOMINMAX
#  define NOMINMAX
#endif

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <ostream>
#include <string>
#include <utility>
#include "future_compat.h"

#ifndef VW_NOEXCEPT
#  include "vw_exception.h"
#endif

#include "memory.h"

// If you get an error message saying that x uses undefined struct 'v_array<...,void>' that means the type
// is not trivially copyable and cannot be used with v_array.
template <typename T, typename Enable = void>
struct v_array;

// v_array makes use of realloc for efficiency. However, it is only safe to use trivially copyable types,
// as std::realloc may do a memcpy if a new piece of memory must be allocated.

template <class T>
struct v_array<T, typename std::enable_if<std::is_trivially_copyable<T>::value>::type>
{
  static_assert(sizeof(T) > 0, "The sizeof v_array's element type T cannot be 0.");

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
    _end_array = nullptr;
    _erase_count = 0;
  }

  void reserve_nocheck(size_t length)
  {
    if (capacity() == length || length == 0) { return; }
    const size_t old_len = size();

    T* temp = reinterpret_cast<T*>(std::realloc(_begin, sizeof(T) * length));
    if (temp == nullptr)
    { THROW_OR_RETURN("realloc of " << length << " failed in reserve_nocheck().  out of memory?"); }
    else
    {
      _begin = temp;
    }

    _end = _begin + std::min(old_len, length);
    _end_array = _begin + length;
    memset(_end, 0, (_end_array - _end) * sizeof(T));
  }

  // This will move all elements after idx by width positions and reallocate the underlying buffer if needed.
  void make_space_at(size_t idx, size_t width)
  {
    _end += width;
    if (size() + width > capacity()) { reserve(2 * capacity() + width); }
    memmove(&_begin[idx + width], &_begin[idx], (size() - (idx + width)) * sizeof(T));
  }

  void resize_no_initialize(size_t old_size, size_t length)
  {
    // if new length is smaller than current size destroy the excess elements
    for (auto idx = length; idx < old_size; ++idx) { _begin[idx].~T(); }
    reserve(length);
    _end = _begin + length;
  }

  void copy_into_this(const v_array<T>& src)
  {
    clear();
    resize_no_initialize(size(), src.size());
    std::copy(src.begin(), src.end(), begin());
  }

  T* _begin;
  T* _end;
  T* _end_array;
  size_t _erase_count;

public:
  using value_type = T;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using iterator = value_type*;
  using const_iterator = const value_type*;

  pointer data() noexcept { return _begin; }
  const_pointer data() const noexcept { return _begin; }

  // enable C++ 11 for loops
  inline iterator begin() noexcept { return _begin; }
  inline iterator end() noexcept { return _end; }

  inline const_iterator begin() const noexcept { return _begin; }
  inline const_iterator end() const noexcept { return _end; }

  inline const_iterator cbegin() const noexcept { return _begin; }
  inline const_iterator cend() const noexcept { return _end; }

  v_array() noexcept : _begin(nullptr), _end(nullptr), _end_array(nullptr), _erase_count(0) {}
  ~v_array() { delete_v_array(); }

  v_array(v_array<T>&& other) noexcept
  {
    _erase_count = 0;
    _begin = nullptr;
    _end = nullptr;
    _end_array = nullptr;

    std::swap(_begin, other._begin);
    std::swap(_end, other._end);
    std::swap(_end_array, other._end_array);
    std::swap(_erase_count, other._erase_count);
  }

  v_array<T>& operator=(v_array<T>&& other) noexcept
  {
    std::swap(_begin, other._begin);
    std::swap(_end, other._end);
    std::swap(_end_array, other._end_array);
    std::swap(_erase_count, other._erase_count);
    return *this;
  }

  v_array(const v_array<T>& other)
  {
    _begin = nullptr;
    _end = nullptr;
    _end_array = nullptr;
    _erase_count = 0;

    copy_into_this(other);
  }

  v_array<T>& operator=(const v_array<T>& other)
  {
    if (this == &other) return *this;

    copy_into_this(other);
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

  VW_DEPRECATED("v_array::pop() is deprecated. Use pop_back(). This will be removed in VW 9.0.")
  T pop()
  {
    T ret = back();
    pop_back();
    return ret;
  }

  VW_DEPRECATED("v_array::last() is deprecated. Use back(). This will be removed in VW 9.0.")
  T last() const { return *(_end - 1); }

  bool empty() const { return _begin == _end; }

  VW_DEPRECATED("v_array::decr() is deprecated. Use pop_back(). This will be removed in VW 9.0.")
  void decr() { _end--; }

  VW_DEPRECATED("v_array::incr() is deprecated. Use push_back(). This will be removed in VW 9.0.")
  void incr()
  {
    if (_end == _end_array) reserve_nocheck(2 * capacity() + 3);
    _end++;
  }
  T& operator[](size_t i) const { return _begin[i]; }
  inline size_t size() const { return _end - _begin; }
  inline size_t capacity() const { return _end_array - _begin; }

  // maintain the original (deprecated) interface for compatibility. To be removed in VW 10
  VW_DEPRECATED(
      "v_array::resize() is deprecated. Use reserve() instead. For standard resize behavior, use "
      "resize_but_with_stl_behavior(). This will be removed in VW 9.0.")
  void resize(size_t length) { reserve_nocheck(length); }

  // change the number of elements in the vector
  // to be renamed to resize() in VW 10
  void resize_but_with_stl_behavior(size_t length)
  {
    auto old_size = size();
    resize_no_initialize(old_size, length);
    // default construct any newly added elements
    // TODO: handle non-default constructable objects
    // requires second interface
    for (auto idx = old_size; idx < length; ++idx) { new (&_begin[idx]) T(); }
  }

  void shrink_to_fit()
  {
    if (size() < capacity())
    {
      if (empty())
      {
        // realloc on size 0 doesn't have a specified behavior
        // just shrink to 1 for now (alternatively, call delete_v())
        reserve_nocheck(1);
      }
      else
      {
        reserve_nocheck(size());
      }
    }
  }

  // reserve enough space for the specified number of elements
  inline void reserve(size_t length)
  {
    if (capacity() < length) reserve_nocheck(length);
  }

  // Don't modify the buffer size, just clear the elements
  inline void clear_noshrink()
  {
    for (T* item = _begin; item != _end; ++item) item->~T();
    _end = _begin;
  }

  void clear()
  {
    if (++_erase_count & ERASE_POINT)
    {
      shrink_to_fit();
      _erase_count = 0;
    }
    clear_noshrink();
  }

  /// \brief Erase item at the given iterator
  /// \param it Iterator to erase at. UB if it is nullptr or out of bounds of the v_array
  /// \returns Iterator to item immediately following the erased element. May be equal to end()
  /// \note Invalidates iterators
  inline iterator erase(iterator it)
  {
    assert(it >= begin());
    assert(it != nullptr);
    assert(it < end());

    const size_t idx = it - begin();
    memmove(&_begin[idx], &_begin[idx + 1], (size() - (idx + 1)) * sizeof(T));
    --_end;
    return begin() + idx;
  }

  /// \brief Erase items from first to end. [first, end)
  /// \param first Iterator to begin erasing at. UB if it is nullptr or out of bounds of the v_array
  /// \param last Iterator to end erasing at. UB if it is nullptr or out of bounds of the v_array
  /// \returns Iterator to item immediately following the erased elements. May be equal to end()
  /// \note Invalidates iterators
  inline iterator erase(iterator first, iterator last)
  {
    assert(first != nullptr);
    assert(last != nullptr);
    assert(first <= last);
    assert(first >= begin());
    assert(first < end());
    assert(last < end());

    const size_t first_index = first - begin();
    const size_t num_to_erase = last - first;
    memmove(
        &_begin[first_index], &_begin[first_index + num_to_erase], (size() - (first_index + num_to_erase)) * sizeof(T));
    _end -= num_to_erase;
    return begin() + first_index;
  }

  /// \brief Insert item into v_array directly after position.
  /// \param first Iterator to insert at. May be end(). UB if outside bounds.
  /// \param elem Element to insert
  /// \returns Iterator to inserted item.
  /// \note Invalidates iterators
  inline iterator insert(iterator it, const T& elem)
  {
    assert(it >= begin());
    assert(it <= end());
    const size_t idx = it - begin();
    make_space_at(idx, 1);
    new (&_begin[idx]) T(elem);
    return _begin + idx;
  }

  /// \brief Insert item into v_array directly after position.
  /// \param first Iterator to insert at. May be end(). UB if outside bounds.
  /// \param elem Element to insert
  /// \returns Iterator to inserted item.
  /// \note Invalidates iterators
  inline iterator insert(iterator it, T&& elem)
  {
    assert(it >= begin());
    assert(it <= end());
    const size_t idx = it - begin();
    make_space_at(idx, 1);
    new (&_begin[idx]) T(std::move(elem));
    return _begin + idx;
  }

  template <class InputIt>
  void insert(iterator it, InputIt first, InputIt last)
  {
    assert(it >= begin());
    assert(it <= end());
    const size_t idx = it - begin();
    const auto num_elements = std::distance(first, last);
    make_space_at(idx, num_elements);
    std::copy(first, last, begin() + idx);
  }

  VW_DEPRECATED("Use destructor instead. This will be removed in VW 9.0.")
  void delete_v() { delete_v_array(); }

  void push_back(const T& new_ele)
  {
    if (_end == _end_array) reserve_nocheck(2 * capacity() + 3);
    new (_end++) T(new_ele);
  }

  void push_back_unchecked(const T& new_ele) { new (_end++) T(new_ele); }

  template <class... Args>
  void emplace_back(Args&&... args)
  {
    if (_end == _end_array) reserve_nocheck(2 * capacity() + 3);
    new (_end++) T(std::forward<Args>(args)...);
  }

  VW_DEPRECATED("Use std::lower_bound instead. This will be removed in VW 9.0.")
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

  VW_DEPRECATED("Use insert and std::upper_bound instead. This will be removed in VW 9.0.")
  size_t unique_add_sorted(const T& new_ele)
  {
    size_t index = 0;
    size_t size = _end - _begin;
    size_t to_move;
    VW_WARNING_STATE_PUSH
    VW_WARNING_DISABLE_DEPRECATED_USAGE
    if (!contain_sorted(new_ele, index))
    {
      if (_end == _end_array) { reserve_nocheck(2 * capacity() + 3); }

      to_move = size - index;

      if (to_move > 0)
      {
        memmove(_begin + index + 1, _begin + index,
            to_move * sizeof(T));  // kopiuje to_move*.. bytow z _begin+index do _begin+index+1
      }

      _begin[index] = new_ele;

      _end++;
    }
    VW_WARNING_STATE_POP

    return index;
  }

  VW_DEPRECATED("Use std::lower_bound instead. This will be removed in VW 9.0.")
  bool contain_sorted(const T& ele, size_t& index)
  {
    VW_WARNING_STATE_PUSH
    VW_WARNING_DISABLE_DEPRECATED_USAGE
    index = find_sorted(ele);
    VW_WARNING_STATE_POP

    if (index == this->size()) return false;

    if (_begin[index] == ele) return true;

    return false;
  }
};

template <class T>
VW_DEPRECATED("Use v_array's constructor directly instead. This will be removed in VW 9.0.")
inline v_array<T> v_init()
{
  return v_array<T>();
}

template <class T>
VW_DEPRECATED("Use v_array's copy constructor or assignment directly instead. This will be removed in VW 9.0.")
void copy_array(v_array<T>& dst, const v_array<T>& src)
{
  dst = src;
}

template <class T>
VW_DEPRECATED("Use v_array's copy constructor or assignment directly instead. This will be removed in VW 9.0.")
void copy_array_no_memcpy(v_array<T>& dst, const v_array<T>& src)
{
  dst.clear();
  for (const auto& item : src) { dst.push_back(item); }
}

template <class T>
VW_DEPRECATED("Use v_array's copy constructor directly instead. This will be removed in VW 9.0.")
void copy_array(v_array<T>& dst, const v_array<T>& src, T (*copy_item)(const T&))
{
  dst.clear();
  for (const auto& item : src) { dst.push_back(copy_item(*item)); }
}

template <class T>
VW_DEPRECATED("Use v_array::insert instead. This will be removed in VW 9.0.")
void push_many(v_array<T>& v, const T* src, size_t num)
{
  v.insert(v.end(), src, src + num);
}

template <class T>
VW_DEPRECATED(
    "calloc_reserve is no longer supported. You should use appropriate constructors instead. This will be removed in "
    "VW 9.0.")
void calloc_reserve(v_array<T>& v, size_t length)
{
  v.clear();
  v.reserve(length);
  std::memset(v.begin(), 0, length * sizeof(T));
}

template <class T>
VW_DEPRECATED("Use std::find instead. This will be removed in VW 9.0.")
bool v_array_contains(const v_array<T>& A, T x)
{
  for (auto e = A.cbegin(); e != A.cend(); ++e)
    if (*e == x) return true;
  return false;
}

template <class T>
std::ostream& operator<<(std::ostream& os, const v_array<T>& v)
{
  os << '[';
  for (auto i = v.cbegin(); i != v.cend(); ++i) os << ' ' << *i;
  os << " ]";
  return os;
}

template <class T, class U>
std::ostream& operator<<(std::ostream& os, const v_array<std::pair<T, U> >& v)
{
  os << '[';
  for (auto i = v.cbegin(); i != v.cend(); ++i) os << ' ' << i->first << ':' << i->second;
  os << " ]";
  return os;
}

VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_DEPRECATED_USAGE

template <class T>
VW_DEPRECATED("pop is deprecated and will be removed in a future version. This will be removed in VW 9.0.")
v_array<T> pop(v_array<v_array<T> >& stack)
{
  if (stack.size() > 0)
  {
    stack.pop_back();
    return stack.back();
  }
  return v_array<T>();
}

VW_DEPRECATED("v_string is deprecated and will be removed in a future version. This will be removed in VW 9.0.")
typedef v_array<unsigned char> v_string;

VW_DEPRECATED("string2v_string is deprecated and will be removed in a future version. This will be removed in VW 9.0.")
inline v_string string2v_string(const std::string& s)
{
  v_string res;
  if (!s.empty()) { res.insert(res.end(), s.begin(), s.end()); }
  return res;
}

VW_DEPRECATED("v_string2string is deprecated and will be removed in a future version. This will be removed in VW 9.0.")
inline std::string v_string2string(const v_string& v_s)
{
  std::string res;
  for (auto i = v_s.cbegin(); i != v_s.cend(); ++i) res.push_back(*i);
  return res;
}

VW_WARNING_STATE_POP
