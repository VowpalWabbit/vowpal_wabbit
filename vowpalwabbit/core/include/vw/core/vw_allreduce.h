// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/allreduce/allreduce.h"
#include "vw/common/vw_exception.h"
#include "vw/core/global_data.h"

#include <cstddef>

namespace VW
{
namespace details
{
template <class T, void (*f)(T&, const T&)>
void all_reduce(VW::workspace& all, T* buffer, const size_t n)
{
  switch (all.selected_all_reduce_type)
  {
    case all_reduce_type::SOCKET:
    {
      auto* all_reduce_sockets_ptr = dynamic_cast<all_reduce_sockets*>(all.all_reduce.get());
      if (all_reduce_sockets_ptr == nullptr) { THROW("all_reduce was not a all_reduce_sockets* object") }
      all_reduce_sockets_ptr->all_reduce<T, f>(buffer, n, all.logger);
      break;
    }
    case all_reduce_type::THREAD:
    {
      auto* all_reduce_threads_ptr = dynamic_cast<all_reduce_threads*>(all.all_reduce.get());
      if (all_reduce_threads_ptr == nullptr) { THROW("all_reduce was not a all_reduce_threads* object") }
      all_reduce_threads_ptr->all_reduce<T, f>(buffer, n);
      break;
    }
  }
}
}  // namespace details
}  // namespace VW
