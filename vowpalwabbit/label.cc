#include "label.h"

std::array<polylabel::destroy_fn, 9> polylabel::_destroy_functions = {
  &polylabel::destroy_unset,
  &polylabel::destroy_empty,
  &polylabel::destroy_simple,
  &polylabel::destroy_multi,
  &polylabel::destroy_cs,
  &polylabel::destroy_cb,
  &polylabel::destroy_conditional_contextual_bandit,
  &polylabel::destroy_cb_eval,
  &polylabel::destroy_multilabels
};

std::array<polylabel::copy_fn, 9> polylabel::_copy_functions = {
  &polylabel::copy_unset,
  &polylabel::copy_empty,
  &polylabel::copy_simple,
  &polylabel::copy_multi,
  &polylabel::copy_cs,
  &polylabel::copy_cb,
  &polylabel::copy_conditional_contextual_bandit,
  &polylabel::copy_cb_eval,
  &polylabel::copy_multilabels
};

std::array<polylabel::move_fn, 9> polylabel::_move_functions = {
  &polylabel::move_unset,
  &polylabel::move_empty,
  &polylabel::move_simple,
  &polylabel::move_multi,
  &polylabel::move_cs,
  &polylabel::move_cb,
  &polylabel::move_conditional_contextual_bandit,
  &polylabel::move_cb_eval,
  &polylabel::move_multilabels
};