#include "example_data_generator.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace vwtest
{

VW::rand_state example_data_generator::create_test_random_state()
{
  const char* test_name = ::testing::UnitTest::GetInstance()->current_test_info()->name();

  VW::rand_state rng(VW::uniform_hash(test_name, std::strlen(test_name), 0));
  return rng;
}

prototype_namespace_t example_data_generator::create_namespace(
    std::string name, uint8_t numeric_features, uint8_t string_features)
{
  std::vector<feature_t> features;
  for (uint8_t i = 0; i < numeric_features; i++)
  {
    features.push_back({"f_" + std::to_string(i), rng.get_and_update_random()});
  }

  for (uint8_t i = 0; i < string_features; i++) { features.push_back({"s_" + std::to_string(i), 1.0f}); }

  return {name.c_str(), features};
}

prototype_example_t example_data_generator::create_simple_example(uint8_t numeric_features, uint8_t string_features)
{
  return {{
              create_namespace("Simple", numeric_features, string_features),
          },
      simple_label(rng.get_and_update_random())};
}

prototype_example_t example_data_generator::create_cb_action(
    uint8_t action, float probability, bool rewarded, const char* tag)
{
  prototype_label_t label =
      probability > 0 ? vwtest::cb_label({rewarded ? -1.0f : 0.0f, action, probability}) : vwtest::no_label();

  return {{
              create_namespace("ActionIds", 0, 4),
              create_namespace("Parameters", 5, 0),
          },
      label, tag};
}

prototype_example_t example_data_generator::create_shared_context(
    uint8_t numeric_features, uint8_t string_features, const char* tag)
{
  return {{
              create_namespace("Shared", numeric_features, string_features),
          },
      cb_label_shared(), tag};
}

prototype_multiexample_t example_data_generator::create_cb_adf_example(
    uint8_t num_actions, uint8_t reward_action_id, const char* tag)
{
  bool rewarded = reward_action_id > 0;
  ssize_t reward_action_index = static_cast<ssize_t>(reward_action_id) - 1;

  std::vector<prototype_example_t> examples;
  examples.push_back(create_shared_context(8, 7, tag));

  for (uint8_t i = 1; i <= num_actions; i++)
  {
    bool action_rewarded = rewarded && i == reward_action_index;
    examples.push_back(create_cb_action(i, 0.5f + (0.5f / num_actions), action_rewarded, tag));
  }

  return {examples};
}

prototype_example_collection_t example_data_generator::create_cb_adf_log(
    uint8_t num_examples, uint8_t num_actions, float reward_p)
{
  std::vector<prototype_multiexample_t> examples;
  for (uint8_t i = 0; i < num_examples; i++)
  {
    uint8_t reward_action_id =
        rng.get_and_update_random() < reward_p ? rng.get_and_update_random() * num_actions + 1 : 0;
    examples.push_back(create_cb_adf_example(num_actions, reward_action_id));
  }

  return {{}, examples, true};
}

prototype_example_collection_t example_data_generator::create_simple_log(
    uint8_t num_examples, uint8_t numeric_features, uint8_t string_features)
{
  std::vector<prototype_example_t> examples;
  for (uint8_t i = 0; i < num_examples; i++)
  {
    examples.push_back(create_simple_example(numeric_features, string_features));
  }

  return {examples, {}, false};
}

}  // namespace vwtest