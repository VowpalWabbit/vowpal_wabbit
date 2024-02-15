// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "flatbuffers/flatbuffers.h"
#include "prototype_example.h"
#include "prototype_example_root.h"
#include "prototype_label.h"
#include "prototype_namespace.h"
#include "vw/common/hash.h"
#include "vw/common/random.h"

#include <vector>

USE_PROTOTYPE_MNEMONICS_EX

namespace vwtest
{

class example_data_generator
{
public:
  example_data_generator() : rng(create_test_random_state()) {}

  static VW::rand_state create_test_random_state();

  prototype_namespace_t create_namespace(std::string name, uint8_t numeric_features, uint8_t string_features);

  prototype_example_t create_simple_example(uint8_t numeric_features, uint8_t string_features);
  prototype_example_t create_cb_action(
      uint8_t action, float probability = 0.0, bool rewarded = false, const char* tag = nullptr);
  prototype_example_t create_shared_context(
      uint8_t numeric_features, uint8_t string_features, const char* tag = nullptr);

  prototype_multiexample_t create_cb_adf_example(
      uint8_t num_actions, uint8_t reward_action_id, const char* tag = nullptr);
  prototype_example_collection_t create_cb_adf_log(uint8_t num_examples, uint8_t num_actions, float reward_p);
  prototype_example_collection_t create_simple_log(
      uint8_t num_examples, uint8_t numeric_features, uint8_t string_features);

private:
  VW::rand_state rng;
};

}  // namespace vwtest