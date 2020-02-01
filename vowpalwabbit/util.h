#pragma once

#include "example.h"
#include "prediction.h"

inline void swap_to_scores(multi_ex& examples)
{
 for (auto& ex : examples)
 {
   auto probs = std::move(ex->pred.action_probs());
   ex->pred.reset();
   ex->pred.init_as_action_scores(std::move(probs));
 }
}

inline void swap_to_probs(multi_ex& examples)
{
 for (auto& ex : examples)
 {
   auto scores = std::move(ex->pred.action_scores());
   ex->pred.reset();
   ex->pred.init_as_action_probs(std::move(scores));
 }
}