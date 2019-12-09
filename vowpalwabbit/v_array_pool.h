#pragma once

#include "v_array.h"
#include "object_pool.h"

namespace VW
{
template <typename T>
struct v_array_allocator
{
  v_array<T> operator()() { return v_init<T>(); }
};

template <typename T>
struct v_array_deleter
{
  void operator()(v_array<T>& array) { array.delete_v(); }
};

template <typename T>
using v_array_pool = VW::value_object_pool<v_array<T>, v_array_allocator<T>, v_array_deleter<T>>;
}  // namespace VW