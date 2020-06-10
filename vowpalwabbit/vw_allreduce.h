// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw.h"
#include "allreduce.h"

template <class T, void (*f)(T&, const T&)>
void all_reduce(vw& all, T* buffer, const size_t n)
{
  switch (all.all_reduce_type)
  {
    case AllReduceType::Socket:
      ((AllReduceSockets*)all.all_reduce)->all_reduce<T, f>(buffer, n);
      break;

    case AllReduceType::Thread:
      ((AllReduceThreads*)all.all_reduce)->all_reduce<T, f>(buffer, n);
      break;
  }
}
