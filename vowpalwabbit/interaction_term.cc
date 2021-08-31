// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "interaction_term.h"

#include "constant.h"

namespace INTERACTIONS
{
namespace_index interaction_term::ns_char() const
{
  // It doesn't make sense to get the character for a wildcard.
  assert(!_wildcard);
  return _ns_char;
}

uint64_t interaction_term::ns_hash() const
{
  // It doesn't make sense to get the hash for a wildcard.
  assert(!_wildcard);
  return _ns_hash;
}

bool interaction_term::wildcard() const { return _wildcard; }

interaction_term_type interaction_term::type() const { return _type; }

bool operator<(const interaction_term& lhs, const interaction_term& rhs)
{
  if (lhs._type != rhs._type) { return lhs._type < rhs._type; }
  if (lhs._ns_char != rhs._ns_char) { return lhs._ns_char < rhs._ns_char; }
  if (lhs._ns_hash != rhs._ns_hash) { return lhs._ns_hash < rhs._ns_hash; }
  return lhs._wildcard < rhs._wildcard;
}

bool operator<=(const interaction_term& lhs, const interaction_term& rhs) { return !(rhs < lhs); }

bool operator>(const interaction_term& lhs, const interaction_term& rhs) { return rhs < lhs; }

bool operator>=(const interaction_term& lhs, const interaction_term& rhs) { return !(lhs < rhs); }

bool operator==(const interaction_term& lhs, const interaction_term& rhs)
{
  // Ignore _ns_hash for char only terms.
  if (lhs._type == interaction_term_type::ns_char)
  { return lhs._ns_char == rhs._ns_char && lhs._wildcard == rhs._wildcard && lhs._type == rhs._type; }

  return lhs._ns_char == rhs._ns_char && lhs._ns_hash == rhs._ns_hash && lhs._wildcard == rhs._wildcard &&
      lhs._type == rhs._type;
}

bool operator!=(const interaction_term& lhs, const interaction_term& rhs) { return !(lhs == rhs); }

interaction_term::interaction_term(namespace_index ns_char) : _ns_char(ns_char), _type(interaction_term_type::ns_char)
{
  assert(_ns_char != wildcard_namespace);
}

interaction_term::interaction_term(namespace_index ns_char, uint64_t ns_hash)
    : _ns_char(ns_char), _ns_hash(ns_hash), _type(interaction_term_type::full_name)
{
  assert(_ns_char != wildcard_namespace);
}

interaction_term::interaction_term(namespace_index ns_char, uint64_t ns_hash, bool wildcard, interaction_term_type type)
    : _ns_char(ns_char), _ns_hash(ns_hash), _wildcard(wildcard), _type(type)
{
}

interaction_term interaction_term::make_wildcard(interaction_term_type type)
{
  return {char{0}, uint64_t{0}, true, type};
}

bool contains_wildcard(const std::vector<interaction_term>& interaction)
{
  return std::find_if(interaction.begin(), interaction.end(),
             [](const interaction_term& term) { return term.wildcard(); }) != interaction.end();
}
}  // namespace INTERACTIONS
