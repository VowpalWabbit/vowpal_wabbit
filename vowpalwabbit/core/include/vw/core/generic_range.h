// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <type_traits>

namespace VW
{
/// generic_range is simply an adapter that given a begin and end iterator acts
/// as an adapter enabling usage in a range based for loop.
template <typename IteratorT, typename dummy = void>
class generic_range
{
};

template <typename IteratorT>
class generic_range<IteratorT, typename std::enable_if<std::is_const<IteratorT>::value>::type>
{
public:
  generic_range(IteratorT begin, IteratorT end) : _begin(begin), _end(end) {}
  IteratorT begin() const { return _begin; }
  IteratorT end() const { return _end; }

private:
  IteratorT _begin;
  IteratorT _end;
};

template <typename IteratorT>
class generic_range<IteratorT, typename std::enable_if<!std::is_const<IteratorT>::value>::type>
{
public:
  generic_range(IteratorT begin, IteratorT end) : _begin(begin), _end(end) {}
  IteratorT begin() { return _begin; }
  IteratorT end() { return _end; }

private:
  IteratorT _begin;
  IteratorT _end;
};

}  // namespace VW
