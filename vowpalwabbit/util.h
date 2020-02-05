#pragma once

#include "example.h"
#include "prediction.h"

inline void swap_to_scores(multi_ex& examples)
{
  for (auto& ex : examples)
  {
    ex->pred.reinterpret(prediction_type_t::action_scores);
  }
}

inline void swap_to_probs(multi_ex& examples)
{
  for (auto& ex : examples)
  {
    ex->pred.reinterpret(prediction_type_t::action_probs);
  }
}