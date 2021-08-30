// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

// for: namespace_index
#include <ostream>

#include "feature_group.h"

namespace INTERACTIONS
{
enum class interaction_term_type
{
  ns_char,
  full_name
};

inline const char* to_string(interaction_term_type e)
{
  switch (e)
  {
    case interaction_term_type::ns_char:
      return "ns_char";
    case interaction_term_type::full_name:
      return "full_name";
    default:
      return "unknown";
  }
}

struct interaction_term
{
  namespace_index ns_char() const;
  uint64_t ns_hash() const;
  bool wildcard() const;
  interaction_term_type type() const;

  friend bool operator<(const interaction_term& lhs, const interaction_term& rhs);

  friend bool operator<=(const interaction_term& lhs, const interaction_term& rhs);
  friend bool operator>(const interaction_term& lhs, const interaction_term& rhs);
  friend bool operator>=(const interaction_term& lhs, const interaction_term& rhs);
  friend bool operator==(const interaction_term& lhs, const interaction_term& rhs);
  friend bool operator!=(const interaction_term& lhs, const interaction_term& rhs);

  explicit interaction_term(namespace_index ns_char);

  interaction_term(namespace_index ns_char, uint64_t ns_hash);

  interaction_term(namespace_index ns_char, uint64_t ns_hash, bool wildcard, interaction_term_type type);

  static interaction_term make_wildcard(interaction_term_type type);

private:
  namespace_index _ns_char = 0;
  uint64_t _ns_hash = 0;
  bool _wildcard = false;
  interaction_term_type _type;
};

bool contains_wildcard(const std::vector<interaction_term>& interaction);
}  // namespace INTERACTIONS
