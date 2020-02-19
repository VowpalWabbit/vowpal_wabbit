#include "prediction.h"

std::array<polyprediction::destroy_fn, 10> polyprediction::_destroy_functions = {
  &polyprediction::destroy_unset,
  &polyprediction::destroy_scalar,
  &polyprediction::destroy_scalars,
  &polyprediction::destroy_action_scores,
  &polyprediction::destroy_multiclassprobs,
  &polyprediction::destroy_multiclass,
  &polyprediction::destroy_multilabels,
  &polyprediction::destroy_prob,
  &polyprediction::destroy_decision_scores,
  &polyprediction::destroy_action_probs
};

std::array<polyprediction::copy_fn, 10> polyprediction::_copy_functions = {
  &polyprediction::copy_unset,
  &polyprediction::copy_scalar,
  &polyprediction::copy_scalars,
  &polyprediction::copy_action_scores,
  &polyprediction::copy_multiclassprobs,
  &polyprediction::copy_multiclass,
  &polyprediction::copy_multilabels,
  &polyprediction::copy_prob,
  &polyprediction::copy_decision_scores,
  &polyprediction::copy_action_probs
};

std::array<polyprediction::move_fn, 10> polyprediction::_move_functions = {
  &polyprediction::move_unset,
  &polyprediction::move_scalar,
  &polyprediction::move_scalars,
  &polyprediction::move_action_scores,
  &polyprediction::move_multiclassprobs,
  &polyprediction::move_multiclass,
  &polyprediction::move_multilabels,
  &polyprediction::move_prob,
  &polyprediction::move_decision_scores,
  &polyprediction::move_action_probs
};
