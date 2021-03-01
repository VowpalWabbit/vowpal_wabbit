// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include <iostream>
#include <vector>
#include <utility>

#include "v_array.h"

namespace std
{
template <class T, class U>
std::ostream& operator<<(std::ostream& os, const std::pair<T, U>& pair)
{
  os << pair.first << ':' << pair.second;
  return os;
}

template <class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec)
{
  os << '[';
  for (const auto& i : vec) os << ' ' << i;
  os << " ]";
  return os;
}
}  // namespace std

template <class T>
std::ostream& operator<<(std::ostream& os, const v_array<T>& v)
{
  os << '[';
  for (T* i = v._begin; i != v._end; ++i) os << ' ' << *i;
  os << " ]";
  return os;
}

template <class T, class U>
std::ostream& operator<<(std::ostream& os, const v_array<std::pair<T, U> >& v)
{
  os << '[';
  for (std::pair<T, U>* i = v._begin; i != v._end; ++i) os << ' ' << i->first << ':' << i->second;
  os << " ]";
  return os;
}
