#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <algorithm>
#include <functional>

namespace base
{
template <typename T, typename _Alloc = std::allocator<T>>
struct vector_view
{
public:
  using iterator = typename std::vector<T, _Alloc>::iterator;
  using const_iterator = typename std::vector<T, _Alloc>::const_iterator;

  vector_view(std::vector<T, _Alloc>& vec) : _(vec) {};

  iterator begin() { return _.begin(); }
  iterator end() { return _.end(); }
  const_iterator begin() const { return _.begin(); }
  const_iterator end() const { return _.end(); }

private:
  std::vector<T, _Alloc>& _;
};

}