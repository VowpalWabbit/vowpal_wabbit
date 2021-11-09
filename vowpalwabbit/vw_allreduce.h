// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <cstddef>

#include "allreduce.h"
#include "global_data.h"
#include "vw_exception.h"

template <class T, void (*f)(T&, const T&)>
void all_reduce(VW::workspace& all, T* buffer, const size_t n)
{
  switch (all.all_reduce_type)
  {
    case AllReduceType::Socket:
    {
      auto* all_reduce_sockets_ptr = dynamic_cast<AllReduceSockets*>(all.all_reduce);
      if (all_reduce_sockets_ptr == nullptr) { THROW("all_reduce was not a AllReduceSockets* object") }
      all_reduce_sockets_ptr->all_reduce<T, f>(buffer, n);
      break;
    }
    case AllReduceType::Thread:
    {
      auto* all_reduce_threads_ptr = dynamic_cast<AllReduceThreads*>(all.all_reduce);
      if (all_reduce_threads_ptr == nullptr) { THROW("all_reduce was not a AllReduceThreads* object") }
      all_reduce_threads_ptr->all_reduce<T, f>(buffer, n);
      break;
    }
  }
}
