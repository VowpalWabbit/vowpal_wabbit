// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/common/future_compat.h"

#include <utility>

namespace VW
{
namespace details
{
template <typename T>
class swap_guard_impl
{
public:
  swap_guard_impl(T* original_location, T* value_to_swap) noexcept
      : _original_location(original_location), _value_to_swap(value_to_swap)
  {
    std::swap(*_original_location, *_value_to_swap);
  }

  swap_guard_impl(const swap_guard_impl&) = delete;
  swap_guard_impl& operator=(const swap_guard_impl&) = delete;
  swap_guard_impl& operator=(swap_guard_impl&& other) = delete;

  swap_guard_impl(swap_guard_impl&& other) noexcept
      : _original_location(other._original_location)
      , _value_to_swap(other._value_to_swap)
      , _will_swap_back(other._will_swap_back)
  {
    other._will_swap_back = false;
    other._original_location = nullptr;
    other._value_to_swap = nullptr;
  }

  ~swap_guard_impl() noexcept { do_swap(); }

  void cancel() noexcept { _will_swap_back = false; }

  /// Returns true if the swap occurred, otherwise false.
  bool do_swap() noexcept
  {
    if (_will_swap_back == true)
    {
      std::swap(*_original_location, *_value_to_swap);
      _will_swap_back = false;
      return true;
    }
    return false;
  }

private:
  T* _original_location;
  T* _value_to_swap;
  bool _will_swap_back = true;
};

template <typename T>
class swap_guard_impl_rvalue
{
public:
  swap_guard_impl_rvalue(T* original_location, T&& value_to_swap) noexcept
      : _original_location(original_location), _value_to_swap(std::move(value_to_swap))
  {
    std::swap(*_original_location, _value_to_swap);
  }

  swap_guard_impl_rvalue(const swap_guard_impl_rvalue&) = delete;
  swap_guard_impl_rvalue& operator=(const swap_guard_impl_rvalue&) = delete;
  swap_guard_impl_rvalue& operator=(swap_guard_impl_rvalue&& other) = delete;

  swap_guard_impl_rvalue(swap_guard_impl_rvalue&& other) noexcept
      : _original_location(other._original_location)
      , _value_to_swap(std::move(other._value_to_swap))
      , _will_swap_back(other._will_swap_back)
  {
    other._will_swap_back = false;
    other._original_location = nullptr;
  }

  ~swap_guard_impl_rvalue() noexcept { do_swap(); }

  void cancel() noexcept { _will_swap_back = false; }

  /// Returns true if the swap occurred, otherwise false.
  bool do_swap() noexcept
  {
    if (_will_swap_back == true)
    {
      std::swap(*_original_location, _value_to_swap);
      _will_swap_back = false;
      return true;
    }
    return false;
  }

private:
  T* _original_location;
  T _value_to_swap;
  bool _will_swap_back = true;
};

}  // namespace details

/// This guard will swap the two locations on creation and upon deletion swap them back.
///
/// #### Example:
/// \code
/// void use_widget(widget& my_widget)
/// {
///   auto new_widget_value = ::get_new_widget_value();
///   auto temp = std::move(my_widget.value);
///   my_widget.value = std::move(new_widget_value);
///
///   do_thing_with_widget(my_widget);
///
///   new_widget_value = std::move(my_widget.value);
///   my_widget.value = std::move(temp);
/// }
///
/// // Can be replaced with:
///
/// void use_widget(widget& my_widget)
/// {
///   auto new_widget_value = ::get_new_widget_value();
///   auto guard = VW::swap_guard(my_widget.value, new_widget_value);
///   do_thing_with_widget(my_widget);
/// }
/// \endcode
template <typename T>
VW_ATTR(nodiscard)
inline details::swap_guard_impl<T> swap_guard(T& original_location, T& value_to_swap) noexcept
{
  return details::swap_guard_impl<T>(&original_location, &value_to_swap);
}

/// This guard will swap the two locations on creation and upon deletion swap them back.
/// Note: This overload allows for a temporary value to be passed in.
///
/// #### Example:
/// \code
/// void use_widget(widget& my_widget)
/// {
///   auto new_widget_value = ::get_new_widget_value();
///   auto temp = std::move(my_widget.value);
///   my_widget.value = std::move(new_widget_value);
///
///   do_thing_with_widget(my_widget);
///
///   new_widget_value = std::move(my_widget.value);
///   my_widget.value = std::move(temp);
/// }
///
/// // Can be replaced with:
///
/// void use_widget(widget& my_widget)
/// {
///   auto guard = VW::swap_guard(my_widget.value, ::get_new_widget_value(););
///   do_thing_with_widget(my_widget);
/// }
/// \endcode
template <typename T>
VW_ATTR(nodiscard)
inline details::swap_guard_impl_rvalue<T> swap_guard(T& original_location, T&& value_to_swap) noexcept
{
  return details::swap_guard_impl_rvalue<T>(&original_location, std::forward<T>(value_to_swap));
}

/// This guard will replace the location with a default constructed object on creation and upon deletion swap the
/// original value back. This guard is equivalent to `swap_guard<T>(xxx, T())`
///
/// #### Example:
/// \code
/// void use_widget(widget& my_widget)
/// {
///   auto new_widget_value = ::get_new_widget_value();
///   auto temp = std::move(my_widget.value);
///   my_widget.value = std::move(new_widget_value);
///
///   do_thing_with_widget(my_widget);
///
///   new_widget_value = std::move(my_widget.value);
///   my_widget.value = std::move(temp);
/// }
///
/// // Can be replaced with:
///
/// void use_widget(widget& my_widget)
/// {
///   auto guard = VW::swap_guard(my_widget.value, ::get_new_widget_value(););
///   do_thing_with_widget(my_widget);
/// }
/// \endcode
template <typename T>
VW_ATTR(nodiscard)
inline details::swap_guard_impl_rvalue<T> stash_guard(T& original_location) noexcept
{
  return details::swap_guard_impl_rvalue<T>(&original_location, std::forward<T>(T()));
}

}  // namespace VW
