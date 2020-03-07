#pragma once

enum class label_type_t
{
  unset,
  empty,
  simple,
  multi,
  cs,       // cost-sensitive
  cb,       // contextual-bandit
  ccb,      // conditional contextual-bandit
  cb_eval,  // contextual-bandit evaluation
  multilabels
};

#define TO_STRING_CASE(enum_type) \
  case enum_type:                 \
    return #enum_type;

inline const char* to_string(label_type_t label_type)
{
  switch (label_type)
  {
    TO_STRING_CASE(label_type_t::unset)
    TO_STRING_CASE(label_type_t::empty)
    TO_STRING_CASE(label_type_t::simple)
    TO_STRING_CASE(label_type_t::multi)
    TO_STRING_CASE(label_type_t::cs)
    TO_STRING_CASE(label_type_t::cb)
    TO_STRING_CASE(label_type_t::ccb)
    TO_STRING_CASE(label_type_t::cb_eval)
    TO_STRING_CASE(label_type_t::multilabels)
    default:
      return "<unknown label type>";
  }
}
