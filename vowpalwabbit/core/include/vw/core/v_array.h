// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/future_compat.h"
#include "vw/common/vw_exception.h"
#include "vw/core/memory.h"

#include <algorithm>
#include <cassert>
#include <ostream>
#include <type_traits>
#include <utility>

namespace VW
{
/**
 * \brief This is a diagnostic overload used to prevent v_array from being used with types that are not trivially
 * copyable.
 * \tparam T Element type
 * \tparam Enable Used to check if T is trivially_copyable
 * \note If you get an error message saying that x uses undefined class 'v_array<...,void>' that means the type is
 * not trivially copyable and cannot be used with v_array.
 */
template <typename T, typename Enable = void>
class v_array;

/**
 * \brief v_array is a container type that makes use of realloc for efficiency.  However, it is only safe to use
 * trivially copyable types, as std::realloc may do a memcpy if a new piece of memory must be allocated.
 * \tparam T Element type
 */
template <class T>
class v_array<T, typename std::enable_if<std::is_trivially_copyable<T>::value>::type>
{
public:
  static_assert(sizeof(T) > 0, "The sizeof v_array's element type T cannot be 0.");

  using value_type = T;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using iterator = value_type*;
  using const_iterator = const value_type*;

  pointer data() noexcept { return _begin; }
  const_pointer data() const noexcept { return _begin; }

  iterator begin() noexcept { return _begin; }
  iterator end() noexcept { return _end; }

  const_iterator begin() const noexcept { return _begin; }
  const_iterator end() const noexcept { return _end; }

  const_iterator cbegin() const noexcept { return _begin; }
  const_iterator cend() const noexcept { return _end; }

  v_array() noexcept : _begin(nullptr), _end(nullptr), _end_array(nullptr) {}
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

  v_array& operator=(v_array<T>&& other) noexcept
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

  v_array& operator=(const v_array<T>& other)
  {
    if (this == &other) { return *this; }

    copy_into_this(other);
    return *this;
  }

  T& back() { return *(_end - 1); }
  const T& back() const { return *(_end - 1); }

  /**
   * \brief Remove the last element from the container. If container is empty then this is undefined behavior.
   */
  void pop_back()
  {
    // Check if the v_array is empty.
    assert(_begin != _end);
    destruct_item(--_end);
  }

  bool empty() const { return _begin == _end; }

  T& operator[](size_t i) const
  {
    assert(i < size());
    return _begin[i];
  }
  size_t size() const
  {
    assert(_end >= _begin);
    return static_cast<size_t>(_end - _begin);
  }
  size_t capacity() const
  {
    assert(_end_array >= _begin);
    return static_cast<size_t>(_end_array - _begin);
  }

  /**
   * \brief Change the size of the container.
   * \param length  Will default construct new elements if length is larger than current or remove elements if it is
   * smaller. \note To be renamed to resize() in VW 9
   */
  VW_DEPRECATED("Renamed to resize. resize_but_with_stl_behavior will be removed in VW 10.")
  void resize_but_with_stl_behavior(size_t length) { resize(length); }

  void resize(size_t length)
  {
    const auto old_size = size();
    resize_no_initialize(old_size, length);
    // default construct any newly added elements
    // TODO: handle non-default constructable objects
    // requires second interface
    for (auto idx = old_size; idx < length; ++idx) { new (&_begin[idx]) T(); }
  }

  /**
   * \brief Shrink the underlying buffer to just be large enough to hold the current elements.
   */
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
      else { reserve_nocheck(size()); }
    }
  }

  /**
   * \brief Reserve enough space for the specified number of elements. If the given size is less than the current
   * capacity this call will do nothing. \param length Ensure the underlying buffer can fit at least this many elements.
   */
  void reserve(size_t length)
  {
    if (capacity() < length) { reserve_nocheck(length); }
  }

  // Don't modify the buffer size, just clear the elements
  void clear_noshrink()
  {
    for (T* item = _begin; item != _end; ++item) { destruct_item(item); }
    _end = _begin;
  }

  /**
   * \brief Clear all elements from container. Additionally keeps track of an erase count and when it reaches a certain
   * threshold it will also shrink the underlying buffer.
   */
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
  iterator erase(iterator it)
  {
    assert(it >= begin());
    assert(it != nullptr);
    assert(it < end());

    const size_t idx = it - begin();
    destruct_item(it);
    std::memmove(&_begin[idx], &_begin[idx + 1], (size() - (idx + 1)) * sizeof(T));
    --_end;
    return begin() + idx;
  }

  /// \brief Erase items from first to end. [first, end)
  /// \param first Iterator to begin erasing at. UB if it is nullptr or out of bounds of the v_array
  /// \param last Iterator to end erasing at. UB if it is nullptr or out of bounds of the v_array
  /// \returns Iterator to item immediately following the erased elements. May be equal to end()
  /// \note Invalidates iterators
  iterator erase(iterator first, iterator last)
  {
    assert(first != nullptr);
    assert(last != nullptr);
    assert(first <= last);
    assert(first >= begin());
    assert(first < end());
    assert(last < end());

    const size_t first_index = first - begin();
    const size_t num_to_erase = last - first;
    for (auto current = first; current != last; ++current) { destruct_item(current); }
    std::memmove(
        &_begin[first_index], &_begin[first_index + num_to_erase], (size() - (first_index + num_to_erase)) * sizeof(T));
    _end -= num_to_erase;
    return begin() + first_index;
  }

  /// \brief Insert item into v_array directly after position.
  /// \param it Iterator to insert at. May be end(). UB if outside bounds.
  /// \param elem Element to insert
  /// \returns Iterator to inserted item.
  /// \note Invalidates iterators
  iterator insert(iterator it, const T& elem)
  {
    assert(it >= begin());
    assert(it <= end());
    const size_t idx = it - begin();
    make_space_at(idx, 1);
    new (&_begin[idx]) T(elem);
    return _begin + idx;
  }

  /// \brief Insert item into v_array directly after position.
  /// \param it Iterator to insert at. May be end(). UB if outside bounds.
  /// \param elem Element to insert
  /// \returns Iterator to inserted item.
  /// \note Invalidates iterators
  iterator insert(iterator it, T&& elem)
  {
    assert(it >= begin());
    assert(it <= end());
    const size_t idx = it - begin();
    make_space_at(idx, 1);
    new (&_begin[idx]) T(std::move(elem));
    return _begin + idx;
  }

  /**
   * \brief Insert the range (first, last] pointed to by first and last into this container at position it.
   * \tparam InputIt Iterator type of input
   * \param it Iterator to insert at. May be end(). UB if outside bounds.
   * \param first iterator to copy from
   * \param last iterator to copy to, but not including
   * \note Invalidates iterators
   */
  template <class InputIt>
  void insert(iterator it, InputIt first, InputIt last)
  {
    assert(it >= begin());
    assert(it <= end());
    const auto idx = static_cast<size_t>(it - begin());
    assert(first <= last);
    const auto num_elements = static_cast<size_t>(std::distance(first, last));
    make_space_at(idx, num_elements);
    std::copy(first, last, begin() + idx);
  }

  /**
   * \brief Add new element to end of container.
   * \param new_ele
   */
  void push_back(const T& new_ele)
  {
    if (_end == _end_array) { reserve_nocheck(2 * capacity() + 3); }
    new (_end++) T(new_ele);
  }

  /**
   * \brief Does not check if the container has the capacity for the new element. UB if size() == capacity(), or if
   * container has just been default constructed as the internal buffers have not been allocated until an item has been
   * inserted.
   * \param new_ele Element to insert
   */
  void push_back_unchecked(const T& new_ele) { new (_end++) T(new_ele); }

  template <class... Args>
  void emplace_back(Args&&... args)
  {
    if (_end == _end_array) { reserve_nocheck(2 * capacity() + 3); }
    new (_end++) T(std::forward<Args>(args)...);
  }

  // Why use hidden friend? https://jacquesheunis.com/post/hidden-friend-compilation/
  friend std::ostream& operator<<(std::ostream& os, const v_array<T>& v)
  {
    os << '[';
    for (auto i = v.cbegin(); i != v.cend(); ++i) { os << ' ' << *i; }
    os << " ]";
    return os;
  }

private:
  static constexpr size_t ERASE_POINT = ~((1u << 10u) - 1u);

  template <typename S, typename std::enable_if<std::is_trivially_destructible<S>::value, bool>::type = true>
  static void destruct_item(S* /* unused */)
  {
    // If S is trivially destructive nothing needs to be done.
  }

  template <typename S, typename std::enable_if<!std::is_trivially_destructible<S>::value, bool>::type = true>
  static void destruct_item(S* ptr)
  {
    ptr->~S();
  }

  void delete_v_array()
  {
    if (_begin != nullptr)
    {
      for (iterator item = _begin; item != _end; ++item) { destruct_item(item); }
      std::free(_begin);
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

    T* temp = static_cast<T*>(std::realloc(_begin, sizeof(T) * length));
    if (temp == nullptr)
    {
      THROW_OR_RETURN("realloc of " << length << " failed in reserve_nocheck().  out of memory?");
    }
    _begin = temp;

    _end = _begin + std::min(old_len, length);
    _end_array = _begin + length;
    assert(_end_array >= _end);
    std::memset(static_cast<void*>(_end), 0, static_cast<size_t>(_end_array - _end) * sizeof(T));
  }

  // This will move all elements after idx by width positions and reallocate the underlying buffer if needed.
  void make_space_at(size_t idx, size_t width)
  {
    if (width > 0)
    {
      if (size() + width > capacity()) { reserve_nocheck(2 * capacity() + width); }
      std::memmove(&_begin[idx + width], &_begin[idx], (size() - idx) * sizeof(T));
      std::memset(&_begin[idx], 0, width * sizeof(T));
      _end += width;
    }
  }

  void resize_no_initialize(size_t old_size, size_t length)
  {
    // if new length is smaller than current size destroy the excess elements
    for (auto idx = length; idx < old_size; ++idx) { destruct_item(&_begin[idx]); }
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
  size_t _erase_count{};
};

}  // namespace VW

//  VW_DEPRECATED: This is deprecated. Cannot mark templates as deprecated though so a message must suffice.
// TODO: remove this alias.
template <typename T>
using v_array = VW::v_array<T>;
