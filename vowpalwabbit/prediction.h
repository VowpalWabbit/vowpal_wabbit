#pragma once

namespace prediction_type
{
enum prediction_type_t
{
  scalar,
  scalars,
  action_scores,
  action_probs,
  multiclass,
  multilabels,
  prob,
  multiclassprobs,
  decision_probs
};

const char* to_string(prediction_type_t prediction_type);


#define CASE(type) \
  case type:       \
    return #type;

inline const char* to_string(prediction_type_t prediction_type)
{
  switch (prediction_type)
  {
    CASE(scalar)
    CASE(scalars)
    CASE(action_scores)
    CASE(action_probs)
    CASE(multiclass)
    CASE(multilabels)
    CASE(prob)
    CASE(multiclassprobs)
    default:
      return "<unsupported>";
  }
}
}  // namespace prediction_type
