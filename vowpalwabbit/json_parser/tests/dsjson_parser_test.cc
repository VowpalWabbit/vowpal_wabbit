// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/conditional_contextual_bandit.h"
#include "vw/test_common/matchers.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

TEST(ParseDsjson, UnderscoreP)
{
  const std::string json_text = R"(
{
  "_p": [0.4, 0.6]
}
  )";
  auto vw = VW::initialize(vwtest::make_args("--dsjson", "--chain_hash", "--cb_adf", "--no_stdin", "--quiet"));
  VW::parsers::json::decision_service_interaction interaction;

  auto examples = vwtest::parse_dsjson(*vw, json_text, &interaction);
  VW::finish_example(*vw, examples);

  static constexpr float EXPECTED_PDF[2] = {0.4f, 0.6f};
  const size_t num_probabilities = interaction.probabilities.size();
  EXPECT_EQ(num_probabilities, 2);
  for (size_t i = 0; i < num_probabilities; ++i)
  {
    // Check that probabilities are as expected.
    EXPECT_EQ(interaction.probabilities[i], EXPECTED_PDF[i]);
  }
}

TEST(ParseDsjson, P)
{
  const std::string json_text = R"(
{
  "p": [0.4, 0.6]
}
  )";
  auto vw = VW::initialize(vwtest::make_args("--dsjson", "--chain_hash", "--cb_adf", "--no_stdin", "--quiet"));
  VW::parsers::json::decision_service_interaction interaction;

  auto examples = vwtest::parse_dsjson(*vw, json_text, &interaction);
  VW::finish_example(*vw, examples);

  static constexpr float EXPECTED_PDF[2] = {0.4f, 0.6f};
  const size_t num_probabilities = interaction.probabilities.size();
  EXPECT_EQ(num_probabilities, 2);
  for (size_t i = 0; i < num_probabilities; ++i)
  {
    // Check that probabilities are as expected.
    EXPECT_EQ(interaction.probabilities[i], EXPECTED_PDF[i]);
  }
}

TEST(ParseDsjson, PDuplicates)
{
  const std::string json_text = R"(
{
  "c": {
    "_p": [0.4, 0.6]
  },
  "p": [0.4, 0.3, 0.3],
  "_p": [0.5, 0.5]
}
  )";
  auto vw = VW::initialize(vwtest::make_args("--dsjson", "--chain_hash", "--cb_adf", "--no_stdin", "--quiet"));
  VW::parsers::json::decision_service_interaction interaction;

  auto examples = vwtest::parse_dsjson(*vw, json_text, &interaction);
  VW::finish_example(*vw, examples);

  // Use the latest "p" or "_p" field provided. The "_p" is ignored when it's inside "c".
  static constexpr float EXPECTED_PDF[2] = {0.5f, 0.5f};
  const size_t num_probabilities = interaction.probabilities.size();
  EXPECT_EQ(num_probabilities, 2);
  for (size_t i = 0; i < num_probabilities; ++i)
  {
    // Check that probabilities are as expected.
    EXPECT_EQ(interaction.probabilities[i], EXPECTED_PDF[i]);
  }
}

TEST(ParseDsjson, PdropFloat)
{
  const std::string json_text = R"(
{
  "pdrop": 0.1
}
  )";
  auto vw = VW::initialize(vwtest::make_args("--dsjson", "--chain_hash", "--cb_adf", "--no_stdin", "--quiet"));
  VW::parsers::json::decision_service_interaction interaction;

  auto examples = vwtest::parse_dsjson(*vw, json_text, &interaction);
  VW::finish_example(*vw, examples);

  EXPECT_FLOAT_EQ(0.1f, interaction.probability_of_drop);
}

TEST(ParseDsjson, PdropUint)
{
  const std::string json_text = R"(
{
  "pdrop": 0
}
  )";
  auto vw = VW::initialize(vwtest::make_args("--dsjson", "--chain_hash", "--cb_adf", "--no_stdin", "--quiet"));
  VW::parsers::json::decision_service_interaction interaction;

  auto examples = vwtest::parse_dsjson(*vw, json_text, &interaction);
  VW::finish_example(*vw, examples);

  EXPECT_FLOAT_EQ(0.0f, interaction.probability_of_drop);
}

// TODO: Make unit test dig out and verify features.
TEST(ParseDsjson, Cb)
{
  std::string json_text = R"(
{
  "_label_cost": -1,
  "_label_probability": 0.8166667,
  "_label_Action": 2,
  "_labelIndex": 1,
  "Version": "1",
  "EventId": "0074434d3a3a46529f65de8a59631939",
  "a": [
    2,
    1,
    3
  ],
  "c": {
    "shared_ns": {
      "shared_feature": 0
    },
    "_multi": [
      {
        "_tag": "tag",
        "ns1": {
          "f1": 1,
          "f2": "strng"
        },
        "ns2": [
          {
            "f3": "value1"
          },
          {
            "ns3": {
              "f4": 0.994963765
            }
          }
        ]
      },
      {
        "_tag": "tag",
        "ns1": {
          "f1": 1,
          "f2": "strng"
        }
      },
      {
        "_tag": "tag",
        "ns1": {
          "f1": 1,
          "f2": "strng"
        }
      }
    ]
  },
  "p": [
    0.816666663,
    0.183333333,
    0.183333333
  ],
  "VWState": {
    "m": "096200c6c41e42bbb879c12830247637/0639c12bea464192828b250ffc389657"
  }
}
)";
  auto vw = VW::initialize(vwtest::make_args("--dsjson", "--chain_hash", "--cb_adf", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_dsjson(*vw, json_text);

  EXPECT_EQ(examples.size(), 4);

  // Shared example
  EXPECT_EQ(examples[0]->l.cb.costs.size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->l.cb.costs[0].probability, -1.f);
  EXPECT_FLOAT_EQ(examples[0]->l.cb.costs[0].cost, FLT_MAX);

  // Action examples
  EXPECT_EQ(examples[1]->l.cb.costs.size(), 0);
  EXPECT_EQ(examples[2]->l.cb.costs.size(), 1);
  EXPECT_EQ(examples[3]->l.cb.costs.size(), 0);

  EXPECT_FLOAT_EQ(examples[2]->l.cb.costs[0].probability, 0.8166667);
  EXPECT_FLOAT_EQ(examples[2]->l.cb.costs[0].cost, -1.0);
  EXPECT_EQ(examples[2]->l.cb.costs[0].action, 2);
  VW::finish_example(*vw, examples);
}

TEST(ParseDsjson, Cats)
{
  std::vector<std::string> features = {"18-25", "4", "C", "0", "1", "2", "15", "M"};
  std::string json_text = R"(
{
  "_label_ca":
  {
    "cost": 0.657567,
    "pdf_value": 6.20426e-05,
    "action": 185.121
  },
  "Version": "1",
  "EventId": "event_id",
  "c": {
    "18-25":1,
    "4":1,
    "C":1,
    "0":1,
    "1":1,
    "2":1,
    "15":1,
    "M":1
  },
  "VWState": {
    "m": "N/A"
  }
}
)";
  auto vw = VW::initialize(vwtest::make_args("--dsjson", "--chain_hash", "--cats", "4", "--min_value=185",
      "--max_value=23959", "--bandwidth", "1", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_dsjson(*vw, json_text);

  EXPECT_EQ(examples.size(), 1);

  EXPECT_EQ(examples[0]->l.cb_cont.costs.size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->l.cb_cont.costs[0].pdf_value, 6.20426e-05);
  EXPECT_FLOAT_EQ(examples[0]->l.cb_cont.costs[0].cost, 0.657567);
  EXPECT_FLOAT_EQ(examples[0]->l.cb_cont.costs[0].action, 185.121);

  auto& space_names = examples[0]->feature_space[' '].space_names;
  EXPECT_EQ(features.size(), space_names.size());
  for (size_t i = 0; i < space_names.size(); i++) { EXPECT_EQ(space_names[i].name, features[i]); }

  VW::finish_example(*vw, examples);
}

TEST(ParseDsjson, CatsNoLabel)
{
  std::vector<std::string> features = {"18-25", "4", "C", "0", "1", "2", "15", "M"};
  std::string json_text = R"(
{
  "Version": "1",
  "EventId": "event_id",
  "c": {
    "18-25":1,
    "4":1,
    "C":1,
    "0":1,
    "1":1,
    "2":1,
    "15":1,
    "M":1
  },
  "VWState": {
    "m": "N/A"
  }
}
)";
  auto vw = VW::initialize(vwtest::make_args("--dsjson", "--chain_hash", "-t", "--cats", "4", "--min_value=185",
      "--max_value=23959", "--bandwidth", "1", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_dsjson(*vw, json_text);

  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(examples[0]->l.cb_cont.costs.size(), 0);

  auto& space_names = examples[0]->feature_space[' '].space_names;
  EXPECT_EQ(features.size(), space_names.size());
  for (size_t i = 0; i < space_names.size(); i++) { EXPECT_EQ(space_names[i].name, features[i]); }

  VW::finish_example(*vw, examples);
}

TEST(ParseDsjson, CatsWValidPdf)
{
  std::vector<std::string> features = {"18-25", "4", "C", "0", "1", "2", "15", "M"};
  std::string json_text = R"(
{
  "Version": "1",
  "EventId": "event_id",
  "pdf": [{"left": 185, "right": 8109.67, "pdf_value": 2.10314e-06},
    {"left": 8109.67, "right": 23959, "pdf_value": 6.20426e-05}],
  "c": {
    "18-25":1,
    "4":1,
    "C":1,
    "0":1,
    "1":1,
    "2":1,
    "15":1,
    "M":1
  },
  "VWState": {
    "m": "N/A"
  }
}
)";
  auto vw = VW::initialize(vwtest::make_args("--dsjson", "--chain_hash", "--cats", "4", "--min_value=185",
      "--max_value=23959", "--bandwidth", "1", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_dsjson(*vw, json_text);

  EXPECT_EQ(examples.size(), 1);
  const auto& reduction_features =
      examples[0]->ex_reduction_features.template get<VW::continuous_actions::reduction_features>();

  EXPECT_EQ(reduction_features.is_pdf_set(), true);
  EXPECT_EQ(reduction_features.is_chosen_action_set(), false);

  EXPECT_EQ(reduction_features.pdf.size(), 2);
  EXPECT_FLOAT_EQ(reduction_features.pdf[0].left, 185.);
  EXPECT_FLOAT_EQ(reduction_features.pdf[0].right, 8109.67);
  EXPECT_FLOAT_EQ(reduction_features.pdf[0].pdf_value, 2.10314e-06);

  EXPECT_FLOAT_EQ(reduction_features.pdf[1].left, 8109.67);
  EXPECT_FLOAT_EQ(reduction_features.pdf[1].right, 23959.);
  EXPECT_FLOAT_EQ(reduction_features.pdf[1].pdf_value, 6.20426e-05);

  auto& space_names = examples[0]->feature_space[' '].space_names;
  EXPECT_EQ(features.size(), space_names.size());
  for (size_t i = 0; i < space_names.size(); i++) { EXPECT_EQ(space_names[i].name, features[i]); }

  VW::finish_example(*vw, examples);
}

TEST(ParseDsjson, CatsWInvalidPdf)
{
  std::vector<std::string> features = {"18-25", "4", "C", "0", "1", "2", "15", "M"};
  std::string json_text = R"(
{
  "Version": "1",
  "EventId": "event_id",
  "pdf": [
    {"left": 185.121}, {"left": 50, "right":50, "pdf_value": 50}
  ],
  "c": {
    "18-25":1,
    "4":1,
    "C":1,
    "0":1,
    "1":1,
    "2":1,
    "15":1,
    "M":1
  },
  "VWState": {
    "m": "N/A"
  }
}
)";
  auto vw = VW::initialize(vwtest::make_args("--dsjson", "--chain_hash", "--cats", "4", "--min_value=185",
      "--max_value=23959", "--bandwidth", "1", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_dsjson(*vw, json_text);

  EXPECT_EQ(examples.size(), 1);

  const auto& reduction_features =
      examples[0]->ex_reduction_features.template get<VW::continuous_actions::reduction_features>();

  EXPECT_EQ(reduction_features.is_pdf_set(), false);
  EXPECT_EQ(reduction_features.is_chosen_action_set(), false);

  auto& space_names = examples[0]->feature_space[' '].space_names;
  EXPECT_EQ(features.size(), space_names.size());
  for (size_t i = 0; i < space_names.size(); i++) { EXPECT_EQ(space_names[i].name, features[i]); }

  VW::finish_example(*vw, examples);
}

TEST(ParseDsjson, CatsChosenAction)
{
  std::vector<std::string> features = {"18-25", "4", "C", "0", "1", "2", "15", "M"};
  std::string json_text = R"(
{
  "Version": "1",
  "EventId": "event_id",
  "pdf": [
    {"chosen_action": 185}
  ],
  "c": {
    "18-25":1,
    "4":1,
    "C":1,
    "0":1,
    "1":1,
    "2":1,
    "15":1,
    "M":1
  },
  "VWState": {
    "m": "N/A"
  }
}
)";
  auto vw = VW::initialize(vwtest::make_args("--dsjson", "--chain_hash", "--cats", "4", "--min_value=185",
      "--max_value=23959", "--bandwidth", "1", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_dsjson(*vw, json_text);

  const auto& reduction_features =
      examples[0]->ex_reduction_features.template get<VW::continuous_actions::reduction_features>();

  EXPECT_EQ(examples.size(), 1);
  EXPECT_EQ(reduction_features.is_pdf_set(), false);
  EXPECT_EQ(reduction_features.is_chosen_action_set(), true);
  EXPECT_FLOAT_EQ(reduction_features.chosen_action, 185.);

  auto& space_names = examples[0]->feature_space[' '].space_names;
  EXPECT_EQ(features.size(), space_names.size());
  for (size_t i = 0; i < space_names.size(); i++) { EXPECT_EQ(space_names[i].name, features[i]); }

  VW::finish_example(*vw, examples);
}

// TODO: Make unit test dig out and verify features.
TEST(ParseDsjson, Ccb)
{
  std::string json_text = R"(
{
  "Timestamp":"timestamp_utc",
  "Version": "1",
  "EventId": "test_id",
  "c":{
      "_multi": [
        {
          "b_": "1",
          "c_": "1",
          "d_": "1"
        },
        {
          "b_": "2",
          "c_": "2",
          "d_": "2"
        }
      ],
      "_slots":[
          {
              "_id": "00eef1eb-2205-4f47",
              "_inc": [1,2],
              "test": 4
          },
          {
              "_id": "set_id",
              "other": 6
          }
      ]
  },
  "_outcomes":[{
      "_label_cost": 2,
      "_o": [],
      "_a": 1,
      "_p": 0.25
    },
    {
      "_label_cost": 4,
      "_o":[],
      "_a": [2, 1],
      "_p": [0.75, 0.25]
    }
  ],
  "VWState": {
    "m": "096200c6c41e42bbb879c12830247637/0639c12bea464192828b250ffc389657"
  }
}
)";

  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--dsjson", "--chain_hash", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_dsjson(*vw, json_text);

  EXPECT_EQ(examples.size(), 5);
  EXPECT_EQ(examples[0]->l.conditional_contextual_bandit.type, VW::ccb_example_type::SHARED);
  EXPECT_EQ(examples[1]->l.conditional_contextual_bandit.type, VW::ccb_example_type::ACTION);
  EXPECT_EQ(examples[2]->l.conditional_contextual_bandit.type, VW::ccb_example_type::ACTION);
  EXPECT_EQ(examples[3]->l.conditional_contextual_bandit.type, VW::ccb_example_type::SLOT);
  EXPECT_EQ(examples[4]->l.conditional_contextual_bandit.type, VW::ccb_example_type::SLOT);

  auto label1 = examples[3]->l.conditional_contextual_bandit;
  EXPECT_EQ(label1.explicit_included_actions.size(), 2);
  EXPECT_EQ(label1.explicit_included_actions[0], 1);
  EXPECT_EQ(label1.explicit_included_actions[1], 2);
  EXPECT_FLOAT_EQ(label1.outcome->cost, 2.f);
  EXPECT_EQ(label1.outcome->probabilities.size(), 1);
  EXPECT_EQ(label1.outcome->probabilities[0].action, 1);
  EXPECT_FLOAT_EQ(label1.outcome->probabilities[0].score, .25f);

  auto label2 = examples[4]->l.conditional_contextual_bandit;
  EXPECT_EQ(label2.explicit_included_actions.size(), 0);
  EXPECT_FLOAT_EQ(label2.outcome->cost, 4.f);
  EXPECT_EQ(label2.outcome->probabilities.size(), 2);
  EXPECT_EQ(label2.outcome->probabilities[0].action, 2);
  EXPECT_FLOAT_EQ(label2.outcome->probabilities[0].score, .75f);
  EXPECT_EQ(label2.outcome->probabilities[1].action, 1);
  EXPECT_FLOAT_EQ(label2.outcome->probabilities[1].score, .25f);
  VW::finish_example(*vw, examples);
}

TEST(ParseDsjson, CbAsCcb)
{
  std::string json_text = R"(
{
  "_label_cost": -1,
  "_label_probability": 0.8166667,
  "_label_Action": 2,
  "_labelIndex": 1,
  "Version": "1",
  "EventId": "0074434d3a3a46529f65de8a59631939",
  "a": [
    2,
    1,
    3
  ],
  "c": {
    "shared_ns": {
      "shared_feature": 0
    },
    "_multi": [
      {
        "_tag": "tag",
        "ns1": {
          "f1": 1,
          "f2": "strng"
        },
        "ns2": [
          {
            "f3": "value1"
          },
          {
            "ns3": {
              "f4": 0.994963765
            }
          }
        ]
      },
      {
        "_tag": "tag",
        "ns1": {
          "f1": 1,
          "f2": "strng"
        }
      },
      {
        "_tag": "tag",
        "ns1": {
          "f1": 1,
          "f2": "strng"
        }
      }
    ]
  },
  "p": [
    0.816666663,
    0.183333333,
    0.183333333
  ],
  "VWState": {
    "m": "096200c6c41e42bbb879c12830247637/0639c12bea464192828b250ffc389657"
  }
}
)";
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--dsjson", "--chain_hash", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_dsjson(*vw, json_text);

  EXPECT_EQ(examples.size(), 5);
  EXPECT_EQ(examples[0]->l.conditional_contextual_bandit.type, VW::ccb_example_type::SHARED);
  EXPECT_EQ(examples[1]->l.conditional_contextual_bandit.type, VW::ccb_example_type::ACTION);
  EXPECT_EQ(examples[2]->l.conditional_contextual_bandit.type, VW::ccb_example_type::ACTION);
  EXPECT_EQ(examples[3]->l.conditional_contextual_bandit.type, VW::ccb_example_type::ACTION);
  EXPECT_EQ(examples[4]->l.conditional_contextual_bandit.type, VW::ccb_example_type::SLOT);

  auto label2 = examples[4]->l.conditional_contextual_bandit;
  EXPECT_EQ(label2.explicit_included_actions.size(), 0);
  EXPECT_FLOAT_EQ(label2.outcome->cost, -1.f);
  EXPECT_EQ(label2.outcome->probabilities.size(), 1);
  EXPECT_EQ(label2.outcome->probabilities[0].action, 1);
  EXPECT_FLOAT_EQ(label2.outcome->probabilities[0].score, 0.8166667f);
  VW::finish_example(*vw, examples);
}

TEST(ParseDsjson, CbWithNan)
{
  std::string json_text = R"(
{
    "_label_cost": "NaN",
    "_label_probability": "NaN",
    "_label_Action": 2,
    "_labelIndex": 1,
    "o": [
        {
            "v": "NaN",
            "EventId": "123",
            "ActionTaken": false
        }
    ],
    "Timestamp": "2020-01-15T16:23:36.8640000Z",
    "Version": "1",
    "EventId": "abc",
    "a": [
        2,
        1,
        0
    ],
    "c": {
        "shared_feature":1.0,
        "_multi": [
            {
                "id": "a"
            },
            {
                "id": "b"
            },
            {
                "id": "c"
            }
        ]
    },
    "p": [
        "NaN",
        "NaN",
        "NaN"
    ]
}
)";

  auto vw = VW::initialize(vwtest::make_args("--dsjson", "--chain_hash", "--cb_adf", "--no_stdin", "--quiet"));
  auto examples = vwtest::parse_dsjson(*vw, json_text);

  EXPECT_EQ(examples.size(), 4);

  // Shared example
  EXPECT_EQ(examples[0]->l.cb.costs.size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->l.cb.costs[0].probability, -1.f);
  EXPECT_FLOAT_EQ(examples[0]->l.cb.costs[0].cost, FLT_MAX);

  // Action examples
  EXPECT_EQ(examples[1]->l.cb.costs.size(), 0);
  EXPECT_EQ(examples[2]->l.cb.costs.size(), 1);
  EXPECT_EQ(examples[3]->l.cb.costs.size(), 0);

  EXPECT_EQ(std::isnan(examples[2]->l.cb.costs[0].probability), true);
  EXPECT_EQ(std::isnan(examples[2]->l.cb.costs[0].cost), true);
  EXPECT_EQ(examples[2]->l.cb.costs[0].action, 2);
  VW::finish_example(*vw, examples);
}

TEST(ParseDsjson, Slates)
{
  std::string json_text = R"(
{
    "_label_cost": 1,
    "_outcomes": [
        {
            "_a": 1,
            "_p": 0.8
        },
        {
            "_a": [0, 1],
            "_p": [0.6, 0.4]
        }
    ],
    "EventId":"test_id",
    "pdrop":0.1,
    "_skipLearn":true,
    "c": {
        "shared_feature": 1.0,
        "_multi": [
            {
                "_slot_id": 0,
                "feature": 1.0,
                "namespace": {
                    "one": 1.0,
                    "test": "string",
                    "array": [
                        1,
                        2,
                        3
                    ],
                    "another": {
                        "test":1.1,
                        "inner_namespac": [
                            {
                                "feature ": "inner "
                            }
                        ]
                    }
                }
            },
            {
                "_slot_id": 0,
                "feature": 1.0
            },
            {
                "_slot_id": 0,
                "feature": 1.0
            },
            {
                "_slot_id": 1,
                "feature": 1.0
            },
            {
                "_slot_id": 1,
                "feature": 1.0
            }
        ],
        "_slots": [
            {
                "feature": 1.0
            },
            {
                "feature": 1.0
            }
        ]
    }
})";

  auto vw = VW::initialize(vwtest::make_args("--slates", "--dsjson", "--chain_hash", "--no_stdin", "--quiet"));
  VW::parsers::json::decision_service_interaction ds_interaction;
  auto examples = vwtest::parse_dsjson(*vw, json_text, &ds_interaction);

  EXPECT_EQ(examples.size(), 8);
  EXPECT_EQ(examples[0]->l.slates.type, VW::slates::example_type::SHARED);
  EXPECT_EQ(examples[1]->l.slates.type, VW::slates::example_type::ACTION);
  EXPECT_EQ(examples[2]->l.slates.type, VW::slates::example_type::ACTION);
  EXPECT_EQ(examples[3]->l.slates.type, VW::slates::example_type::ACTION);
  EXPECT_EQ(examples[4]->l.slates.type, VW::slates::example_type::ACTION);
  EXPECT_EQ(examples[5]->l.slates.type, VW::slates::example_type::ACTION);
  EXPECT_EQ(examples[6]->l.slates.type, VW::slates::example_type::SLOT);
  EXPECT_EQ(examples[7]->l.slates.type, VW::slates::example_type::SLOT);

  const auto& label0 = examples[0]->l.slates;
  EXPECT_FLOAT_EQ(label0.cost, 1.f);
  EXPECT_EQ(label0.labeled, true);

  EXPECT_EQ(examples[1]->l.slates.slot_id, 0);
  EXPECT_EQ(examples[2]->l.slates.slot_id, 0);
  EXPECT_EQ(examples[3]->l.slates.slot_id, 0);
  EXPECT_EQ(examples[4]->l.slates.slot_id, 1);
  EXPECT_EQ(examples[5]->l.slates.slot_id, 1);

  const auto& label6 = examples[6]->l.slates;
  EXPECT_THAT(label6.probabilities, ::testing::Pointwise(ActionScoreEqual(), std::vector<VW::action_score>{{1, 0.8f}}));

  const auto& label7 = examples[7]->l.slates;
  EXPECT_THAT(label7.probabilities,
      ::testing::Pointwise(ActionScoreEqual(), std::vector<VW::action_score>{{0, 0.6f}, {1, 0.4f}}));

  // Check values in VW::parsers::json::decision_service_interaction
  EXPECT_EQ(ds_interaction.event_id, "test_id");
  EXPECT_FLOAT_EQ(ds_interaction.probability_of_drop, 0.1);
  EXPECT_EQ(ds_interaction.skip_learn, true);
  EXPECT_THAT(ds_interaction.actions, ::testing::ElementsAre(1, 0));
  EXPECT_THAT(ds_interaction.probabilities, ::testing::ElementsAre(0.8f, 0.6f));

  VW::finish_example(*vw, examples);
}

TEST(ParseDsjson, SlatesDomParser)
{
  std::string json_text = R"(
{
    "c": {
        "aFloatFeature": 1.0,
        "aStringFeature": "value",
        "dArray": [
            1,
            2.0,
            {
                "aIntFeature": 5,
                "aNamespace": {
                    "bIntFeature": 1
                }
            }
        ],
        "bNamespace": {
            "cIntFeature": 1,
            "cNamespace": {
                "aBoolFeature": true
            }
        },
        "eNamespace": {
            "bBoolFeature": false
        },
        "_multi": [],
        "_slots": []
    }
}
)";

  // Assert parsed values against what they should be
  auto slates_vw = VW::initialize(vwtest::make_args("--slates", "--dsjson", "--chain_hash", "--no_stdin", "--quiet"));
  auto slates_examples = vwtest::parse_dsjson(*slates_vw, json_text);

  EXPECT_EQ(slates_examples.size(), 1);
  const auto& slates_ex = *slates_examples[0];
  EXPECT_THAT(slates_ex.indices, ::testing::ElementsAre('a', 'd', 'c', 'b', 32));
  EXPECT_EQ(slates_ex.feature_space[' '].indices.size(), 2);
  EXPECT_EQ(slates_ex.feature_space['a'].indices.size(), 1);
  EXPECT_EQ(slates_ex.feature_space['b'].indices.size(), 1);
  EXPECT_EQ(slates_ex.feature_space['c'].indices.size(), 1);
  EXPECT_EQ(slates_ex.feature_space['d'].indices.size(), 3);
  EXPECT_EQ(slates_ex.feature_space['3'].indices.size(), 0);

  // Compare the DOM parser to parsing the same features with the CCB SAX parser
  auto ccb_vw =
      VW::initialize(vwtest::make_args("--ccb_explore_adf", "--dsjson", "--chain_hash", "--no_stdin", "--quiet"));
  auto ccb_examples = vwtest::parse_dsjson(*ccb_vw, json_text);
  EXPECT_EQ(ccb_examples.size(), 1);
  const auto& ccb_ex = *ccb_examples[0];
  EXPECT_THAT(slates_ex.feature_space[' '].indices, ::testing::ElementsAreArray(ccb_ex.feature_space[' '].indices));
  EXPECT_THAT(slates_ex.feature_space['a'].indices, ::testing::ElementsAreArray(ccb_ex.feature_space['a'].indices));
  EXPECT_THAT(slates_ex.feature_space['b'].indices, ::testing::ElementsAreArray(ccb_ex.feature_space['b'].indices));
  EXPECT_THAT(slates_ex.feature_space['c'].indices, ::testing::ElementsAreArray(ccb_ex.feature_space['c'].indices));
  EXPECT_THAT(slates_ex.feature_space['d'].indices, ::testing::ElementsAreArray(ccb_ex.feature_space['d'].indices));
  EXPECT_THAT(slates_ex.feature_space['e'].indices, ::testing::ElementsAreArray(ccb_ex.feature_space['e'].indices));

  EXPECT_THAT(slates_ex.feature_space[' '].values, ::testing::ElementsAreArray(ccb_ex.feature_space[' '].values));
  EXPECT_THAT(slates_ex.feature_space['a'].values, ::testing::ElementsAreArray(ccb_ex.feature_space['a'].values));
  EXPECT_THAT(slates_ex.feature_space['b'].values, ::testing::ElementsAreArray(ccb_ex.feature_space['b'].values));
  EXPECT_THAT(slates_ex.feature_space['c'].values, ::testing::ElementsAreArray(ccb_ex.feature_space['c'].values));
  EXPECT_THAT(slates_ex.feature_space['d'].values, ::testing::ElementsAreArray(ccb_ex.feature_space['d'].values));
  EXPECT_THAT(slates_ex.feature_space['e'].values, ::testing::ElementsAreArray(ccb_ex.feature_space['e'].values));

  VW::finish_example(*slates_vw, slates_examples);
  VW::finish_example(*ccb_vw, ccb_examples);
}
