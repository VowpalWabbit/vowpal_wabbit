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
