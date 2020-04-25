// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <set>

// Performs unique operation over collection without requiring the collection to be sorted first.
// Returns pointer to first element after unique elements to erase from until end of collection.
// Original ordering of elements is preserved.
template <typename ForwardIterator>
ForwardIterator stable_unique(ForwardIterator begin, ForwardIterator end)
{
  using value_t = typename std::iterator_traits<ForwardIterator>::value_type;

  std::set<value_t> unique_set;

  auto current_head = begin;
  for (auto current_check = begin; current_check != end; current_check++)
  {
    if (unique_set.find(*current_check) == unique_set.end())
    {
      unique_set.insert(*current_check);
      *current_head = *current_check;
      current_head++;
    }
  }

  return current_head;
}
