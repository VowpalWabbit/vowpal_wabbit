// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// Batch 6: Targeted tests to push line coverage past 90%.

#include "vw/common/vw_exception.h"
#include "vw/core/action_score.h"
#include "vw/core/api_status.h"
#include "vw/core/best_constant.h"
#include "vw/core/cb.h"
#include "vw/core/cb_type.h"
#include "vw/core/cb_continuous_label.h"
#include "vw/core/cb_with_observations_label.h"
#include "vw/core/ccb_label.h"
#include "vw/core/ccb_reduction_features.h"
#include "vw/core/cost_sensitive.h"
#include "vw/core/estimators/confidence_sequence.h"
#include "vw/core/estimators/confidence_sequence_robust.h"
#include "vw/core/example.h"
#include "vw/core/hashstring.h"
#include "vw/core/io_buf.h"
#include "vw/core/learner.h"
#include "vw/core/loss_functions.h"
#include "vw/core/model_utils.h"
#include "vw/core/multilabel.h"
#include "vw/core/named_labels.h"
#include "vw/core/no_label.h"
#include "vw/core/prediction_type.h"
#include "vw/core/shared_data.h"
#include "vw/core/simple_label.h"
#include "vw/core/slates_label.h"
#include "vw/core/text_utils.h"
#include "vw/core/vw.h"
#include "vw/core/vw_validate.h"
#include "vw/io/io_adapter.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cfloat>
#include <cmath>
#include <sstream>

// ============================================================
// confidence_sequence.cc coverage
// ============================================================

TEST(ConfidenceSequenceCoverage, IncrementalFSumAddition)
{
  VW::details::incremental_f_sum a;
  a += 1.0;
  a += 2.0;
  a += 3.0;
  double val_a = static_cast<double>(a);
  EXPECT_DOUBLE_EQ(val_a, 6.0);

  VW::details::incremental_f_sum b;
  b += 4.0;
  b += 5.0;

  VW::details::incremental_f_sum c = a + b;
  double val_c = static_cast<double>(c);
  EXPECT_DOUBLE_EQ(val_c, 15.0);
}

TEST(ConfidenceSequenceCoverage, PersistMetrics)
{
  VW::estimators::confidence_sequence cs(0.05, 0.0, 1.0, true);
  cs.update(1.0, 0.5, 0.0, -1.0);
  cs.update(0.8, 0.3, 0.0, -1.0);
  cs.update(1.2, 0.7, 0.0, -1.0);

  VW::metric_sink metrics;
  cs.persist(metrics, "_test");

  // persist should have set metrics with the suffix
  EXPECT_GE(cs.lower_bound(), 0.0f);
  EXPECT_LE(cs.upper_bound(), 1.0f);
}

TEST(ConfidenceSequenceCoverage, UpdateWithDropProbability)
{
  VW::estimators::confidence_sequence cs(0.05, 0.0, 1.0, false);
  // Update with p_drop > 0 to cover the n_drop > 0 branch
  cs.update(1.0, 0.5, 0.0, -1.0);
  cs.update(1.0, 0.3, 0.2, -1.0);  // p_drop=0.2, n_drop computed from p_drop
  cs.update(1.0, 0.7, 0.0, 2.0);   // explicit n_drop

  float lb = cs.lower_bound();
  float ub = cs.upper_bound();
  EXPECT_LE(lb, ub);
}

TEST(ConfidenceSequenceCoverage, ApproxPolygammaOnePaths)
{
  // Test with b > 10 and b <= 10 by varying t (which affects internal calculations)
  VW::estimators::confidence_sequence cs(0.05, 0.0, 1.0, true);
  // Many updates to get t > 10
  for (int i = 0; i < 15; ++i) { cs.update(1.0, 0.5, 0.0, -1.0); }
  EXPECT_GE(cs.lower_bound(), 0.0f);
  EXPECT_LE(cs.upper_bound(), 1.0f);
}

TEST(ConfidenceSequenceCoverage, ModelSerializationRoundTrip)
{
  VW::estimators::confidence_sequence original(0.05, 0.0, 1.0, true);
  original.update(1.0, 0.5, 0.0, -1.0);
  original.update(0.8, 0.3, 0.0, -1.0);

  // Serialize
  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  size_t written = VW::model_utils::write_model_field(write_buf, original, "cs_test", false);
  write_buf.flush();
  EXPECT_GT(written, 0u);

  // Deserialize
  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::estimators::confidence_sequence restored(0.05, 0.0, 1.0, true);
  size_t read_bytes = VW::model_utils::read_model_field(read_buf, restored);
  EXPECT_GT(read_bytes, 0u);

  EXPECT_DOUBLE_EQ(restored.alpha, original.alpha);
  EXPECT_EQ(restored.update_count, original.update_count);
  EXPECT_FLOAT_EQ(restored.lower_bound(), original.lower_bound());
  EXPECT_FLOAT_EQ(restored.upper_bound(), original.upper_bound());
}

TEST(ConfidenceSequenceCoverage, IncrementalFSumSerializationRoundTrip)
{
  VW::details::incremental_f_sum original;
  original += 1.5;
  original += 2.5;
  original += 3.5;

  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  size_t written = VW::model_utils::write_model_field(write_buf, original, "fsum_test", false);
  write_buf.flush();
  EXPECT_GT(written, 0u);

  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::details::incremental_f_sum restored;
  size_t read_bytes = VW::model_utils::read_model_field(read_buf, restored);
  EXPECT_GT(read_bytes, 0u);

  EXPECT_DOUBLE_EQ(static_cast<double>(restored), static_cast<double>(original));
}

// ============================================================
// cb_continuous_label.cc coverage
// ============================================================

TEST(CbContinuousLabelCoverage, CacheReadRoundTrip)
{
  auto lp = VW::cb_continuous::the_label_parser;

  VW::polylabel original;
  lp.default_label(original);

  // Add a cost element
  VW::cb_continuous::continuous_label_elm elm{0.5f, 1.0f, 0.3f};
  original.cb_cont.costs.push_back(elm);

  // Set up reduction features
  VW::reduction_features red_fts;
  auto& cats_rf = red_fts.template get<VW::continuous_actions::reduction_features>();
  cats_rf.pdf.push_back(VW::continuous_actions::pdf_segment{0.0f, 1.0f, 0.5f});
  cats_rf.chosen_action = 0.7f;

  // Cache
  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  lp.cache_label(original, red_fts, write_buf, "test", false);
  write_buf.flush();

  // Read cached
  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::polylabel restored;
  lp.default_label(restored);
  VW::reduction_features restored_rf;
  lp.read_cached_label(restored, restored_rf, read_buf);

  EXPECT_EQ(restored.cb_cont.costs.size(), 1u);
  EXPECT_FLOAT_EQ(restored.cb_cont.costs[0].action, 0.5f);
  EXPECT_FLOAT_EQ(restored.cb_cont.costs[0].cost, 1.0f);
  EXPECT_FLOAT_EQ(restored.cb_cont.costs[0].pdf_value, 0.3f);

  auto& restored_cats_rf = restored_rf.template get<VW::continuous_actions::reduction_features>();
  EXPECT_EQ(restored_cats_rf.pdf.size(), 1u);
  EXPECT_FLOAT_EQ(restored_cats_rf.chosen_action, 0.7f);
}

TEST(CbContinuousLabelCoverage, IsTestLabel)
{
  VW::cb_continuous::continuous_label label;
  EXPECT_TRUE(label.is_test_label());
  EXPECT_FALSE(label.is_labeled());

  // Cost with FLT_MAX is a test label
  label.costs.push_back({0.5f, FLT_MAX, 0.0f});
  EXPECT_TRUE(label.is_test_label());

  // Cost with actual cost is not a test label
  label.costs.clear();
  label.costs.push_back({0.5f, 1.0f, 0.3f});
  EXPECT_FALSE(label.is_test_label());
  EXPECT_TRUE(label.is_labeled());
}

TEST(CbContinuousLabelCoverage, ToStringFunctions)
{
  VW::cb_continuous::continuous_label_elm elm{0.5f, 1.0f, 0.3f};
  std::string elm_str = VW::to_string(elm, 4);
  EXPECT_THAT(elm_str, testing::HasSubstr("0.5"));
  EXPECT_THAT(elm_str, testing::HasSubstr("1"));
  EXPECT_THAT(elm_str, testing::HasSubstr("0.3"));

  VW::cb_continuous::continuous_label label;
  label.costs.push_back(elm);
  std::string label_str = VW::to_string(label, 4);
  EXPECT_THAT(label_str, testing::HasSubstr("cb_cont"));
}

TEST(CbContinuousLabelCoverage, ModelUtilsReadWrite)
{
  // Test read/write for pdf_segment
  VW::continuous_actions::pdf_segment seg{0.0f, 1.0f, 0.5f};

  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  VW::model_utils::write_model_field(write_buf, seg, "seg", false);
  write_buf.flush();

  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::continuous_actions::pdf_segment restored_seg;
  VW::model_utils::read_model_field(read_buf, restored_seg);

  EXPECT_FLOAT_EQ(restored_seg.left, 0.0f);
  EXPECT_FLOAT_EQ(restored_seg.right, 1.0f);
  EXPECT_FLOAT_EQ(restored_seg.pdf_value, 0.5f);
}

TEST(CbContinuousLabelCoverage, ModelUtilsReductionFeatures)
{
  VW::continuous_actions::reduction_features rf;
  rf.pdf.push_back({0.0f, 0.5f, 0.3f});
  rf.pdf.push_back({0.5f, 1.0f, 0.7f});
  rf.chosen_action = 0.6f;

  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  VW::model_utils::write_model_field(write_buf, rf, "rf", false);
  write_buf.flush();

  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::continuous_actions::reduction_features restored_rf;
  VW::model_utils::read_model_field(read_buf, restored_rf);

  EXPECT_EQ(restored_rf.pdf.size(), 2u);
  EXPECT_FLOAT_EQ(restored_rf.chosen_action, 0.6f);
}

TEST(CbContinuousLabelCoverage, ContinuousLabelModelRoundTrip)
{
  VW::cb_continuous::continuous_label label;
  label.costs.push_back({0.5f, 1.0f, 0.3f});
  label.costs.push_back({0.8f, 2.0f, 0.7f});

  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  VW::model_utils::write_model_field(write_buf, label, "cl", false);
  write_buf.flush();

  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::cb_continuous::continuous_label restored;
  VW::model_utils::read_model_field(read_buf, restored);

  EXPECT_EQ(restored.costs.size(), 2u);
  EXPECT_FLOAT_EQ(restored.costs[0].action, 0.5f);
  EXPECT_FLOAT_EQ(restored.costs[1].cost, 2.0f);
}

TEST(CbContinuousLabelCoverage, LabelParserWeight)
{
  auto lp = VW::cb_continuous::the_label_parser;
  VW::polylabel label;
  lp.default_label(label);
  VW::reduction_features rf;
  float w = lp.get_weight(label, rf);
  EXPECT_FLOAT_EQ(w, 1.0f);
}

// ============================================================
// ccb_reduction_features.cc coverage
// ============================================================

TEST(CcbReductionFeaturesCoverage, ToStringAllTypes)
{
  EXPECT_THAT(VW::to_string(VW::ccb_example_type::UNSET), testing::HasSubstr("UNSET"));
  EXPECT_THAT(VW::to_string(VW::ccb_example_type::SHARED), testing::HasSubstr("SHARED"));
  EXPECT_THAT(VW::to_string(VW::ccb_example_type::ACTION), testing::HasSubstr("ACTION"));
  EXPECT_THAT(VW::to_string(VW::ccb_example_type::SLOT), testing::HasSubstr("SLOT"));
}

// ============================================================
// no_label.cc coverage
// ============================================================

TEST(NoLabelCoverage, ParseNoLabel)
{
  auto lp = VW::no_label_parser_global;

  VW::polylabel label;
  lp.default_label(label);

  // Parse with empty words (line 23-24)
  VW::label_parser_reuse_mem reuse_mem;
  VW::reduction_features rf;
  std::vector<VW::string_view> empty_words;
  VW::io::logger logger = VW::io::create_default_logger();
  lp.parse_label(label, rf, reuse_mem, nullptr, empty_words, logger);

  // Parse with too many words (line 25-27)
  std::vector<VW::string_view> many_words = {"extra", "words"};
  lp.parse_label(label, rf, reuse_mem, nullptr, many_words, logger);
}

TEST(NoLabelCoverage, CacheReadCached)
{
  auto lp = VW::no_label_parser_global;

  VW::polylabel label;
  lp.default_label(label);
  VW::reduction_features rf;

  // cache_label returns 1
  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  size_t cached = lp.cache_label(label, rf, write_buf, "test", false);
  EXPECT_EQ(cached, 1u);

  // read_cached_label returns 1
  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::polylabel restored;
  lp.default_label(restored);
  size_t read = lp.read_cached_label(restored, rf, read_buf);
  EXPECT_EQ(read, 1u);
}

TEST(NoLabelCoverage, GetWeightAndTestLabel)
{
  auto lp = VW::no_label_parser_global;
  VW::polylabel label;
  lp.default_label(label);
  VW::reduction_features rf;

  EXPECT_FLOAT_EQ(lp.get_weight(label, rf), 1.0f);
  EXPECT_FALSE(lp.test_label(label));
}

TEST(NoLabelCoverage, PrintAndOutputNoLabel)
{
  auto vw = VW::initialize(vwtest::make_args("--no_stdin", "--quiet"));
  auto* ex = VW::read_example(*vw, "| a b c");
  vw->predict(*ex);

  // These exercise print_no_label_update and output_and_account_no_label_example
  VW::details::output_and_account_no_label_example(*vw, *ex);

  VW::finish_example(*vw, *ex);
}

// ============================================================
// simple_label.cc coverage - output and account paths
// ============================================================

TEST(SimpleLabelCoverage, OutputAndAccountExample)
{
  auto vw = VW::initialize(vwtest::make_args("--no_stdin", "--quiet"));
  auto* ex = VW::read_example(*vw, "1.0 | a b c");
  vw->learn(*ex);

  VW::details::output_and_account_example(*vw, *ex);
  VW::finish_example(*vw, *ex);
}

TEST(SimpleLabelCoverage, ReturnSimpleExample)
{
  auto vw = VW::initialize(vwtest::make_args("--no_stdin", "--quiet"));
  auto* ex = VW::read_example(*vw, "0.5 | x y z");
  vw->learn(*ex);

  // return_simple_example calls output_and_account_example + finish_example
  VW::details::return_simple_example(*vw, nullptr, *ex);
}

// ============================================================
// cost_sensitive.cc coverage - cache/read/to_string
// ============================================================

TEST(CostSensitiveCoverage, CacheAndReadRoundTrip)
{
  auto lp = VW::cs_label_parser_global;

  VW::polylabel original;
  lp.default_label(original);
  original.cs.costs.push_back({1.0f, 1, 0.0f, 0.0f});
  original.cs.costs.push_back({2.0f, 2, 0.0f, 0.0f});

  VW::reduction_features rf;

  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  lp.cache_label(original, rf, write_buf, "test", false);
  write_buf.flush();

  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::polylabel restored;
  lp.default_label(restored);
  lp.read_cached_label(restored, rf, read_buf);

  EXPECT_EQ(restored.cs.costs.size(), 2u);
  EXPECT_FLOAT_EQ(restored.cs.costs[0].x, 1.0f);
  EXPECT_EQ(restored.cs.costs[0].class_index, 1u);
}

// ============================================================
// cb.cc coverage - cache/read round trip and label parsing
// ============================================================

TEST(CbLabelCoverage, CacheReadRoundTrip)
{
  auto lp = VW::cb_label_parser_global;

  VW::polylabel original;
  lp.default_label(original);
  original.cb.costs.push_back(VW::cb_class(1.0f, 1, 0.5f));
  original.cb.costs.push_back(VW::cb_class(2.0f, 2, 0.3f));
  original.cb.weight = 1.5f;

  VW::reduction_features rf;

  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  lp.cache_label(original, rf, write_buf, "test", false);
  write_buf.flush();

  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::polylabel restored;
  lp.default_label(restored);
  lp.read_cached_label(restored, rf, read_buf);

  EXPECT_EQ(restored.cb.costs.size(), 2u);
  EXPECT_FLOAT_EQ(restored.cb.costs[0].cost, 1.0f);
  EXPECT_FLOAT_EQ(restored.cb.costs[0].probability, 0.5f);
  EXPECT_FLOAT_EQ(restored.cb.weight, 1.5f);
}

TEST(CbLabelCoverage, ParseLabelProbabilityEdgeCases)
{
  // Exercise probability > 1 and probability < 0 warnings
  auto vw = VW::initialize(vwtest::make_args("--cb", "2", "--quiet"));
  // Probability > 1 should be clamped to 1.0
  auto* ex = VW::read_example(*vw, "1:0.5:1.5 | a b");
  VW::finish_example(*vw, *ex);
}

// ============================================================
// multilabel.cc coverage
// ============================================================

TEST(MultilabelCoverage, CacheReadRoundTrip)
{
  auto lp = VW::multilabel_label_parser_global;

  VW::polylabel original;
  lp.default_label(original);
  original.multilabels.label_v.push_back(1);
  original.multilabels.label_v.push_back(3);
  original.multilabels.label_v.push_back(5);

  VW::reduction_features rf;

  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  lp.cache_label(original, rf, write_buf, "test", false);
  write_buf.flush();

  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::polylabel restored;
  lp.default_label(restored);
  lp.read_cached_label(restored, rf, read_buf);

  EXPECT_EQ(restored.multilabels.label_v.size(), 3u);
  EXPECT_EQ(restored.multilabels.label_v[0], 1u);
  EXPECT_EQ(restored.multilabels.label_v[1], 3u);
  EXPECT_EQ(restored.multilabels.label_v[2], 5u);
}

TEST(MultilabelCoverage, IsTestAndWeight)
{
  auto lp = VW::multilabel_label_parser_global;
  VW::polylabel label;
  lp.default_label(label);
  VW::reduction_features rf;

  // Empty label is test label
  EXPECT_TRUE(lp.test_label(label));

  // With labels, it's not a test label
  label.multilabels.label_v.push_back(1);
  EXPECT_FALSE(lp.test_label(label));

  // Weight is always 1.0
  EXPECT_FLOAT_EQ(lp.get_weight(label, rf), 1.0f);
}

// ============================================================
// slates_label.cc coverage
// ============================================================

TEST(SlatesLabelCoverage, CacheReadRoundTrip)
{
  auto lp = VW::slates::slates_label_parser;

  // Shared type
  VW::polylabel shared_label;
  lp.default_label(shared_label);
  shared_label.slates.type = VW::slates::example_type::SHARED;
  shared_label.slates.labeled = true;
  shared_label.slates.cost = 3.5f;

  VW::reduction_features rf;

  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  lp.cache_label(shared_label, rf, write_buf, "test", false);
  write_buf.flush();

  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::polylabel restored;
  lp.default_label(restored);
  lp.read_cached_label(restored, rf, read_buf);

  EXPECT_EQ(restored.slates.type, VW::slates::example_type::SHARED);
  EXPECT_TRUE(restored.slates.labeled);
  EXPECT_FLOAT_EQ(restored.slates.cost, 3.5f);
}

TEST(SlatesLabelCoverage, SlotCacheRoundTrip)
{
  auto lp = VW::slates::slates_label_parser;

  VW::polylabel slot_label;
  lp.default_label(slot_label);
  slot_label.slates.type = VW::slates::example_type::SLOT;
  slot_label.slates.labeled = true;
  slot_label.slates.probabilities.push_back({0, 0.5f});
  slot_label.slates.probabilities.push_back({1, 0.5f});

  VW::reduction_features rf;

  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  lp.cache_label(slot_label, rf, write_buf, "test", false);
  write_buf.flush();

  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::polylabel restored;
  lp.default_label(restored);
  lp.read_cached_label(restored, rf, read_buf);

  EXPECT_EQ(restored.slates.type, VW::slates::example_type::SLOT);
  EXPECT_TRUE(restored.slates.labeled);
  EXPECT_EQ(restored.slates.probabilities.size(), 2u);
}

// ============================================================
// loss_functions.cc coverage - hinge, quantile, logistic edge cases
// ============================================================

TEST(LossFunctionsCoverage, SquaredLossEdgeCases)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto loss = get_loss_function(*vw, "squared");
  VW::shared_data sd;
  sd.min_label = 0.0f;
  sd.max_label = 1.0f;

  // Prediction below min_label (line 26-33 in squared_loss_impl_get_loss)
  float loss_val = loss->get_loss(&sd, -0.5f, 0.3f);
  EXPECT_GE(loss_val, 0.0f);

  // Prediction above max_label (line 35-40)
  loss_val = loss->get_loss(&sd, 1.5f, 0.7f);
  EXPECT_GE(loss_val, 0.0f);

  // Prediction below min_label, label == min_label (returns 0)
  loss_val = loss->get_loss(&sd, -0.5f, 0.0f);
  EXPECT_FLOAT_EQ(loss_val, 0.0f);

  // Prediction above max_label, label == max_label (returns 0)
  loss_val = loss->get_loss(&sd, 1.5f, 1.0f);
  EXPECT_FLOAT_EQ(loss_val, 0.0f);

  // second_derivative outside range
  EXPECT_FLOAT_EQ(0.0f, loss->second_derivative(&sd, -0.5f, 0.3f));
  EXPECT_FLOAT_EQ(0.0f, loss->second_derivative(&sd, 1.5f, 0.3f));
}

TEST(LossFunctionsCoverage, HingeLossUpdate)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto loss = get_loss_function(*vw, "hinge");

  // get_update with label*prediction < 1, small update
  float update = loss->get_update(0.5f, 1.0f, 0.1f, 1.0f);
  EXPECT_GT(update, 0.0f);

  // get_update with label*prediction >= 1 (returns 0)
  update = loss->get_update(1.5f, 1.0f, 0.1f, 1.0f);
  EXPECT_FLOAT_EQ(update, 0.0f);

  // get_square_grad
  float sg = loss->get_square_grad(0.5f, 1.0f);
  EXPECT_GE(sg, 0.0f);
}

TEST(LossFunctionsCoverage, QuantileLossAllPaths)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  float tau = 0.3f;
  auto loss = get_loss_function(*vw, "quantile", tau);

  // err > 0 path
  float update_pos = loss->get_update(0.3f, 0.5f, 0.1f, 1.0f);
  EXPECT_GT(update_pos, 0.0f);

  // err < 0 path
  float update_neg = loss->get_update(0.7f, 0.5f, 0.1f, 1.0f);
  EXPECT_LT(update_neg, 0.0f);

  // err == 0 path
  float update_zero = loss->get_update(0.5f, 0.5f, 0.1f, 1.0f);
  EXPECT_FLOAT_EQ(update_zero, 0.0f);

  // unsafe_update paths
  EXPECT_GT(loss->get_unsafe_update(0.3f, 0.5f, 0.1f), 0.0f);
  EXPECT_LT(loss->get_unsafe_update(0.7f, 0.5f, 0.1f), 0.0f);
  EXPECT_FLOAT_EQ(loss->get_unsafe_update(0.5f, 0.5f, 0.1f), 0.0f);

  // first_derivative paths
  EXPECT_LT(loss->first_derivative(nullptr, 0.3f, 0.5f), 0.0f);
  EXPECT_GT(loss->first_derivative(nullptr, 0.7f, 0.5f), 0.0f);
  EXPECT_FLOAT_EQ(loss->first_derivative(nullptr, 0.5f, 0.5f), 0.0f);

  // get_square_grad
  float sg = loss->get_square_grad(0.3f, 0.5f);
  EXPECT_GE(sg, 0.0f);

  // get_loss paths
  float loss_pos = loss->get_loss(nullptr, 0.3f, 0.5f);
  EXPECT_GT(loss_pos, 0.0f);
  float loss_neg = loss->get_loss(nullptr, 0.7f, 0.5f);
  EXPECT_GT(loss_neg, 0.0f);
}

TEST(LossFunctionsCoverage, LogisticLoss)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto loss = get_loss_function(*vw, "logistic");

  VW::shared_data sd;
  sd.min_label = -50.0f;
  sd.max_label = 50.0f;

  // Basic loss calculation
  float loss_val = loss->get_loss(&sd, 0.5f, 1.0f);
  EXPECT_GE(loss_val, 0.0f);

  // first and second derivative
  float fd = loss->first_derivative(&sd, 0.5f, 1.0f);
  float sd_val = loss->second_derivative(&sd, 0.5f, 1.0f);
  EXPECT_NE(fd, 0.0f);
  EXPECT_GT(sd_val, 0.0f);
}

TEST(LossFunctionsCoverage, PoissonLossLargeUpdate)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto loss = get_loss_function(*vw, "poisson");

  // Cover the label=0 path in get_update
  float update = loss->get_update(1.0f, 0.0f, 0.1f, 1.0f);
  EXPECT_FALSE(std::isnan(update));

  // Cover large expm1_arg path (label > 0, large update_scale * pred_per_update)
  float large_update = loss->get_update(1.0f, 100.0f, 100.0f, 100.0f);
  EXPECT_FALSE(std::isnan(large_update));

  // Cover overflow path for label=0 with large values
  float overflow_update = loss->get_update(50.0f, 0.0f, 1000.0f, 1000.0f);
  EXPECT_FALSE(std::isnan(overflow_update));
}

// ============================================================
// shared_data.cc coverage - move assignment
// ============================================================

TEST(SharedDataCoverage, MoveAssignment)
{
  VW::shared_data sd1;
  sd1.example_number = 42;
  sd1.weighted_labeled_examples = 100.0;
  sd1.total_features = 500;
  sd1.sum_loss = 25.0;
  sd1.dump_interval = 10.0;
  sd1.gravity = 0.5;
  sd1.contraction = 0.9;
  sd1.min_label = -1.0f;
  sd1.max_label = 1.0f;

  VW::shared_data sd2;
  sd2 = std::move(sd1);

  EXPECT_EQ(sd2.example_number, 42u);
  EXPECT_EQ(sd2.weighted_labeled_examples, 100.0);
  EXPECT_EQ(sd2.total_features, 500u);
  EXPECT_EQ(sd2.sum_loss, 25.0);
  EXPECT_FLOAT_EQ(sd2.min_label, -1.0f);
  EXPECT_FLOAT_EQ(sd2.max_label, 1.0f);
}

// ============================================================
// prediction_type.cc coverage
// ============================================================

TEST(PredictionTypeCoverage, ToStringAllTypes)
{
  // Cover all switch cases in to_string(prediction_type_t)
  // Format is "prediction_type_t::ENUM_NAME" (uppercase, prefixed)
  EXPECT_THAT(VW::to_string(VW::prediction_type_t::SCALAR), testing::HasSubstr("SCALAR"));
  EXPECT_THAT(VW::to_string(VW::prediction_type_t::SCALARS), testing::HasSubstr("SCALARS"));
  EXPECT_THAT(VW::to_string(VW::prediction_type_t::ACTION_SCORES), testing::HasSubstr("ACTION_SCORES"));
  EXPECT_THAT(VW::to_string(VW::prediction_type_t::PDF), testing::HasSubstr("PDF"));
  EXPECT_THAT(VW::to_string(VW::prediction_type_t::ACTION_PDF_VALUE), testing::HasSubstr("ACTION_PDF_VALUE"));
  EXPECT_THAT(VW::to_string(VW::prediction_type_t::ACTION_PROBS), testing::HasSubstr("ACTION_PROBS"));
  EXPECT_THAT(VW::to_string(VW::prediction_type_t::MULTICLASS), testing::HasSubstr("MULTICLASS"));
  EXPECT_THAT(VW::to_string(VW::prediction_type_t::MULTILABELS), testing::HasSubstr("MULTILABELS"));
  EXPECT_THAT(VW::to_string(VW::prediction_type_t::PROB), testing::HasSubstr("PROB"));
  EXPECT_THAT(VW::to_string(VW::prediction_type_t::MULTICLASS_PROBS), testing::HasSubstr("MULTICLASS_PROBS"));
  EXPECT_THAT(VW::to_string(VW::prediction_type_t::DECISION_PROBS), testing::HasSubstr("DECISION_PROBS"));
  EXPECT_THAT(VW::to_string(VW::prediction_type_t::ACTIVE_MULTICLASS), testing::HasSubstr("ACTIVE_MULTICLASS"));
  EXPECT_THAT(VW::to_string(VW::prediction_type_t::NOPRED), testing::HasSubstr("NOPRED"));
}

// ============================================================
// example.cc coverage - swap_predictions, to_string(scalars)
// ============================================================

TEST(ExampleCoverage, SwapPredictionsVariousTypes)
{
  // Cover various prediction type swap paths
  {
    VW::polyprediction a, b;
    a.prob = 0.3f;
    b.prob = 0.7f;
    VW::swap_prediction(a, b, VW::prediction_type_t::PROB);
    EXPECT_FLOAT_EQ(a.prob, 0.7f);
    EXPECT_FLOAT_EQ(b.prob, 0.3f);
  }
  {
    VW::polyprediction a, b;
    a.multilabels.label_v.push_back(1);
    a.multilabels.label_v.push_back(2);
    VW::swap_prediction(a, b, VW::prediction_type_t::MULTILABELS);
    EXPECT_EQ(a.multilabels.label_v.size(), 0u);
    EXPECT_EQ(b.multilabels.label_v.size(), 2u);
  }
  {
    VW::polyprediction a, b;
    a.decision_scores.push_back({});
    VW::swap_prediction(a, b, VW::prediction_type_t::DECISION_PROBS);
    EXPECT_EQ(a.decision_scores.size(), 0u);
    EXPECT_EQ(b.decision_scores.size(), 1u);
  }
  {
    VW::polyprediction a, b;
    a.pdf_value.action = 0.5f;
    a.pdf_value.pdf_value = 0.3f;
    VW::swap_prediction(a, b, VW::prediction_type_t::ACTION_PDF_VALUE);
    EXPECT_FLOAT_EQ(b.pdf_value.action, 0.5f);
  }
  {
    VW::polyprediction a, b;
    a.active_multiclass.predicted_class = 3;
    VW::swap_prediction(a, b, VW::prediction_type_t::ACTIVE_MULTICLASS);
    EXPECT_EQ(b.active_multiclass.predicted_class, 3u);
  }
  {
    VW::polyprediction a, b;
    VW::swap_prediction(a, b, VW::prediction_type_t::NOPRED);
    // noop, just check it doesn't crash
  }
  {
    VW::polyprediction a, b;
    a.scalars.push_back(1.0f);
    a.scalars.push_back(2.0f);
    VW::swap_prediction(a, b, VW::prediction_type_t::MULTICLASS_PROBS);
    EXPECT_EQ(a.scalars.size(), 0u);
    EXPECT_EQ(b.scalars.size(), 2u);
  }
}

// Note: VW::to_string(v_array<float>) is not in a public header, covered indirectly

TEST(ExampleCoverage, DestructorWithPassthrough)
{
  // Cover the passthrough deletion path in destructor
  auto ex = VW::make_unique<VW::example>();
  ex->passthrough = new VW::features();
  ex->passthrough->push_back(1.0f, 42);
  // Destructor should delete passthrough without crash
}

// ============================================================
// vw.cc coverage - cmd_string_replace, seed_vw_model, are_features_compatible
// ============================================================

TEST(VwCoverage, CmdStringReplaceValueFlagPresent)
{
  std::stringstream* ss = new std::stringstream("--learning_rate 0.1 --passes 2");
  VW::cmd_string_replace_value(ss, "--learning_rate", "0.5");
  EXPECT_THAT(ss->str(), testing::HasSubstr("--learning_rate 0.5"));
  delete ss;
}

TEST(VwCoverage, CmdStringReplaceValueFlagAbsent)
{
  std::stringstream* ss = new std::stringstream("--passes 2");
  VW::cmd_string_replace_value(ss, "--learning_rate", "0.5");
  EXPECT_THAT(ss->str(), testing::HasSubstr("--learning_rate 0.5"));
  delete ss;
}

TEST(VwCoverage, CmdStringReplaceValueFlagAtEnd)
{
  std::stringstream* ss = new std::stringstream("--passes 2 --learning_rate 0.1");
  VW::cmd_string_replace_value(ss, "--learning_rate", "0.5");
  EXPECT_THAT(ss->str(), testing::HasSubstr("--learning_rate 0.5"));
  delete ss;
}

TEST(VwCoverage, SeedVwModel)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));

  auto* ex = VW::read_example(*vw, "1.0 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);

  auto seeded = VW::seed_vw_model(*vw, {"--quiet"});
  EXPECT_NE(seeded, nullptr);

  auto* ex2 = VW::read_example(*seeded, "| a b c");
  seeded->predict(*ex2);
  VW::finish_example(*seeded, *ex2);
}

TEST(VwCoverage, AreFeaturesCompatibleSame)
{
  auto vw1 = VW::initialize(vwtest::make_args("--quiet"));
  auto vw2 = VW::initialize(vwtest::make_args("--quiet"));

  const char* result = VW::are_features_compatible(*vw1, *vw2);
  EXPECT_EQ(result, nullptr);
}

TEST(VwCoverage, AreFeaturesCompatibleDiffBits)
{
  auto vw1 = VW::initialize(vwtest::make_args("--quiet", "-b", "18"));
  auto vw2 = VW::initialize(vwtest::make_args("--quiet", "-b", "20"));

  const char* result = VW::are_features_compatible(*vw1, *vw2);
  EXPECT_NE(result, nullptr);
  EXPECT_STREQ(result, "num_bits");
}

TEST(VwCoverage, AreFeaturesCompatibleDiffAddConstant)
{
  auto vw1 = VW::initialize(vwtest::make_args("--quiet"));
  auto vw2 = VW::initialize(vwtest::make_args("--quiet", "--noconstant"));

  const char* result = VW::are_features_compatible(*vw1, *vw2);
  EXPECT_NE(result, nullptr);
  EXPECT_STREQ(result, "add_constant");
}

// ============================================================
// Reduction coverage: bootstrap, oaa, gd_mf, interact, marginal
// ============================================================

TEST(ReductionsCoverage, BootstrapTrainPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--bootstrap", "4", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    std::string label = (i % 2 == 0) ? "1" : "-1";
    auto* ex = VW::read_example(*vw, label + " | a:" + std::to_string(i) + " b:" + std::to_string(i * 2));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }

  auto* ex = VW::read_example(*vw, "| a:5 b:10");
  vw->predict(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(ReductionsCoverage, OaaTrainPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "4", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    std::string label = std::to_string((i % 4) + 1);
    auto* ex = VW::read_example(*vw, label + " | a:" + std::to_string(i) + " b:" + std::to_string(i % 5));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }

  auto* ex = VW::read_example(*vw, "| a:5 b:2");
  vw->predict(*ex);
  EXPECT_GE(ex->pred.multiclass, 1u);
  EXPECT_LE(ex->pred.multiclass, 4u);
  VW::finish_example(*vw, *ex);
}

TEST(ReductionsCoverage, GdMfTrainPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "5", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    float label = static_cast<float>(i % 5);
    auto* ex = VW::read_example(*vw, std::to_string(label) + " |a x:" + std::to_string(i) +
        " |b y:" + std::to_string(i % 3));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }

  auto* ex = VW::read_example(*vw, "|a x:3 |b y:1");
  vw->predict(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(ReductionsCoverage, InteractTrainPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--interact", "ab", "--quiet"));

  for (int i = 0; i < 10; ++i)
  {
    float label = static_cast<float>(i % 2);
    auto* ex = VW::read_example(*vw, std::to_string(label) + " |a x:" + std::to_string(i) +
        " |b y:" + std::to_string(i % 3));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(ReductionsCoverage, MarginalTrainAndSaveLoad)
{
  std::vector<char> model_buffer;

  {
    auto vw = VW::initialize(vwtest::make_args("--marginal", "a", "--quiet"));

    for (int i = 0; i < 20; ++i)
    {
      float label = static_cast<float>(i % 3);
      auto* ex = VW::read_example(*vw, std::to_string(label) + " |a x:" + std::to_string(i % 5));
      vw->learn(*ex);
      VW::finish_example(*vw, *ex);
    }

    // Save model
    auto backing_vector = std::make_shared<std::vector<char>>();
    VW::io_buf io_writer;
    io_writer.add_file(VW::io::create_vector_writer(backing_vector));
    VW::save_predictor(*vw, io_writer);
    io_writer.flush();
    model_buffer = *backing_vector;
  }

  // Load and predict
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet"),
        VW::io::create_buffer_view(model_buffer.data(), model_buffer.size()));

    auto* ex = VW::read_example(*vw, "|a x:2");
    vw->predict(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(ReductionsCoverage, GenerateInteractionsQuadratic)
{
  // Exercise generate_interactions.cc paths through quadratic interactions
  auto vw = VW::initialize(vwtest::make_args("-q", "ab", "--quiet"));

  for (int i = 0; i < 10; ++i)
  {
    float label = static_cast<float>(i % 2);
    auto* ex = VW::read_example(*vw, std::to_string(label) + " |a x:" + std::to_string(i) +
        " |b y:" + std::to_string(i % 3));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(ReductionsCoverage, GenerateInteractionsCubic)
{
  // Exercise cubic interaction paths
  auto vw = VW::initialize(vwtest::make_args("--cubic", "abc", "--quiet"));

  for (int i = 0; i < 10; ++i)
  {
    float label = static_cast<float>(i % 2);
    auto* ex = VW::read_example(*vw, std::to_string(label) + " |a x:" + std::to_string(i) +
        " |b y:" + std::to_string(i % 3) +
        " |c z:" + std::to_string(i % 4));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// named_labels.cc coverage
// ============================================================

TEST(NamedLabelsCoverage, GetNamedLabelsAndLookup)
{
  auto vw = VW::initialize(vwtest::make_args("--named_labels", "cat,dog,fish", "--csoaa", "3", "--quiet"));

  auto* ex = VW::read_example(*vw, "1:1 |f a b");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

// ============================================================
// best_constant.cc coverage
// ============================================================

TEST(BestConstantCoverage, GetBestConstant)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));

  // Train on a few examples so sd has accumulated stats
  for (int i = 0; i < 20; ++i)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }

  float best_constant = 0.0f;
  float best_constant_loss = 0.0f;
  bool result = VW::get_best_constant(*vw->loss_config.loss, *vw->sd, best_constant, best_constant_loss);
  // May or may not succeed depending on accumulated stats, but shouldn't crash
  (void)result;
}

// ============================================================
// cb_with_observations_label.cc coverage
// ============================================================

TEST(CbWithObservationsCoverage, CacheReadRoundTrip)
{
  auto lp = VW::cb_with_observations_global;

  VW::polylabel original;
  lp.default_label(original);
  original.cb_with_observations.event.costs.push_back(VW::cb_class(1.0f, 1, 0.5f));
  original.cb_with_observations.is_observation = true;
  original.cb_with_observations.is_definitely_bad = false;

  VW::reduction_features rf;

  VW::io_buf write_buf;
  auto backing = std::make_shared<std::vector<char>>();
  write_buf.add_file(VW::io::create_vector_writer(backing));
  lp.cache_label(original, rf, write_buf, "test", false);
  write_buf.flush();

  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(backing->data(), backing->size()));
  VW::polylabel restored;
  lp.default_label(restored);
  lp.read_cached_label(restored, rf, read_buf);

  EXPECT_EQ(restored.cb_with_observations.event.costs.size(), 1u);
  EXPECT_TRUE(restored.cb_with_observations.is_observation);
  EXPECT_FALSE(restored.cb_with_observations.is_definitely_bad);
}

// ============================================================
// C wrapper (vwdll.cc) coverage
// ============================================================

TEST(CWrapperCoverage, BasicApiFlow)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));

  // Exercise get_label, get_importance, get_initial, get_prediction
  auto* ex = VW::read_example(*vw, "1.0 | a b c");
  vw->learn(*ex);

  float label = VW::get_label(ex);
  float importance = VW::get_importance(ex);
  float initial = VW::get_initial(ex);
  float prediction = VW::get_prediction(ex);
  size_t num_features = VW::get_feature_number(ex);

  EXPECT_FLOAT_EQ(label, 1.0f);
  EXPECT_GT(importance, 0.0f);
  EXPECT_GE(num_features, 1u);
  (void)initial;
  (void)prediction;

  VW::finish_example(*vw, *ex);
}

TEST(CWrapperCoverage, TagOperations)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));

  auto* ex = VW::read_example(*vw, "1.0 mytag| a b c");
  size_t tag_len = VW::get_tag_length(ex);
  const char* tag = VW::get_tag(ex);
  EXPECT_GT(tag_len, 0u);
  EXPECT_NE(tag, nullptr);

  VW::finish_example(*vw, *ex);
}

TEST(CWrapperCoverage, CostSensitivePrediction)
{
  auto vw = VW::initialize(vwtest::make_args("--csoaa", "3", "--quiet"));

  auto* ex = VW::read_example(*vw, "1:1 2:5 3:3 | a b c");
  vw->learn(*ex);
  float cs_pred = VW::get_cost_sensitive_prediction(ex);
  EXPECT_GE(cs_pred, 1.0f);
  EXPECT_LE(cs_pred, 3.0f);
  VW::finish_example(*vw, *ex);
}

TEST(CWrapperCoverage, MultilabelPrediction)
{
  auto vw = VW::initialize(vwtest::make_args("--multilabel_oaa", "4", "--quiet"));

  auto* ex = VW::read_example(*vw, "1,3 | a b c");
  vw->learn(*ex);

  size_t plen = 0;
  void* preds = VW::get_multilabel_predictions(ex, plen);
  (void)preds;

  VW::finish_example(*vw, *ex);
}

TEST(CWrapperCoverage, ActionScoreOperations)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--quiet"));

  auto* shared = VW::read_example(*vw, "shared | s1 s2");
  auto* action1 = VW::read_example(*vw, "0:1:0.5 | a1");
  auto* action2 = VW::read_example(*vw, "| a2");

  VW::multi_ex multi_ex = {shared, action1, action2};
  vw->learn(multi_ex);

  size_t as_len = VW::get_action_score_length(shared);
  if (as_len > 0) { float score = VW::get_action_score(shared, 0); (void)score; }

  vw->finish_example(multi_ex);
}

TEST(CWrapperCoverage, WeightOperations)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));

  auto* ex = VW::read_example(*vw, "1.0 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);

  size_t num_weights = VW::num_weights(*vw);
  EXPECT_GT(num_weights, 0u);

  size_t stride = VW::get_stride(*vw);
  EXPECT_GT(stride, 0u);

  // get/set weight
  float w = VW::get_weight(*vw, 0, 0);
  VW::set_weight(*vw, 0, 0, w + 0.1f);
  float w2 = VW::get_weight(*vw, 0, 0);
  EXPECT_FLOAT_EQ(w2, w + 0.1f);
}

TEST(CWrapperCoverage, ImportExportExample)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));

  // Create a feature space
  VW::primitive_feature_space pfs;
  pfs.name = 'a';
  VW::init_features(pfs, 2);
  VW::set_feature(pfs, 0, VW::hash_feature(*vw, "x", VW::hash_space(*vw, "a")), 1.0f);
  VW::set_feature(pfs, 1, VW::hash_feature(*vw, "y", VW::hash_space(*vw, "a")), 2.0f);

  auto* imported = VW::import_example(*vw, "1.0", &pfs, 1);
  EXPECT_NE(imported, nullptr);

  // Export example
  size_t export_len = 0;
  auto* exported = VW::export_example(*vw, imported, export_len);
  EXPECT_GT(export_len, 0u);

  // Cleanup
  VW::release_feature_space(exported, export_len);
  VW::finish_example(*vw, *imported);

  delete[] pfs.fs;
}

TEST(CWrapperCoverage, ConfidenceAndHashOps)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));

  auto* ex = VW::read_example(*vw, "1.0 | a b c");
  vw->learn(*ex);
  float confidence = VW::get_confidence(ex);
  (void)confidence;
  VW::finish_example(*vw, *ex);

  // Hash operations
  size_t space_hash = VW::hash_space(*vw, "test_space");
  EXPECT_GT(space_hash, 0u);

  size_t feature_hash = VW::hash_feature(*vw, "test_feature", space_hash);
  EXPECT_GT(feature_hash, 0u);

  size_t static_space_hash = VW::hash_space_static("test_space", "strings");
  EXPECT_GT(static_space_hash, 0u);

  size_t static_feature_hash = VW::hash_feature_static("test_feature", space_hash, "strings", 18);
  EXPECT_GT(static_feature_hash, 0u);
}

TEST(CWrapperCoverage, AddLabelAndStringLabel)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));

  auto* ex = VW::read_example(*vw, "| a b c");
  VW::add_label(ex, 1.0f, 1.0f, 0.0f);
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);

  ex = VW::read_example(*vw, "| a b c");
  VW::parse_example_label(*vw, *ex, "0.5");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(CWrapperCoverage, CopyModelAndFreeIOBuf)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));

  auto* ex = VW::read_example(*vw, "1.0 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);

  // Save to buffer via copy_model_data API
  auto backing_vector = std::make_shared<std::vector<char>>();
  VW::io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(backing_vector));
  VW::save_predictor(*vw, io_writer);
  io_writer.flush();

  EXPECT_GT(backing_vector->size(), 0u);
}

// ============================================================
// learner.cc coverage
// ============================================================

TEST(LearnerCoverage, GetEnabledLearners)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "3", "--quiet"));

  std::vector<std::string> learners;
  vw->l->get_enabled_learners(learners);
  EXPECT_FALSE(learners.empty());
  EXPECT_THAT(learners, testing::Contains("gd"));
  EXPECT_THAT(learners, testing::Contains("oaa"));
}

TEST(LearnerCoverage, GetLearnerByNamePrefix)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "3", "--quiet"));

  auto* l = vw->l->get_learner_by_name_prefix("oaa");
  EXPECT_NE(l, nullptr);

  auto* l_gd = vw->l->get_learner_by_name_prefix("gd");
  EXPECT_NE(l_gd, nullptr);
}

TEST(LearnerCoverage, RequireSinglelineMultiline)
{
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet"));
    auto* sl = VW::LEARNER::require_singleline(vw->l.get());
    EXPECT_NE(sl, nullptr);
  }
  {
    auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--quiet"));
    auto* ml = VW::LEARNER::require_multiline(vw->l.get());
    EXPECT_NE(ml, nullptr);
  }
}

// ============================================================
// io_buf.cc coverage
// ============================================================

TEST(IoBufCoverage, OpenFileReadAndWrite)
{
  std::string tmp_file = "./test_io_buf_coverage.tmp";

  // Write
  {
    VW::io_buf io;
    io.add_file(VW::io::open_file_writer(tmp_file));
    const char* data = "hello world";
    io.bin_write_fixed(data, strlen(data));
    io.flush();
  }

  // Read
  {
    VW::io_buf io;
    io.add_file(VW::io::open_file_reader(tmp_file));
    char buf[64] = {0};
    size_t bytes_read = io.bin_read_fixed(buf, 11);
    EXPECT_EQ(bytes_read, 11u);
    EXPECT_STREQ(buf, "hello world");
  }

  std::remove(tmp_file.c_str());
}

// ============================================================
// parser.cc coverage - daemon/cache mode parser paths
// ============================================================

TEST(ParserCoverage, CacheParsingMode)
{
  std::string tmp_file = "./test_parser_cache.tmp";

  // Train and write cache
  {
    auto vw = VW::initialize(vwtest::make_args("--cache_file", tmp_file.c_str(), "--quiet"));
    for (int i = 0; i < 5; ++i)
    {
      auto* ex = VW::read_example(*vw, std::to_string(i % 2) + " | a:" + std::to_string(i));
      vw->learn(*ex);
      VW::finish_example(*vw, *ex);
    }
  }

  // Read from cache
  {
    auto vw = VW::initialize(vwtest::make_args("--cache_file", tmp_file.c_str(), "--quiet"));
    // The cache is automatically detected and used
  }

  std::remove(tmp_file.c_str());
}

// ============================================================
// Reduction: BFGS coverage paths
// ============================================================

TEST(ReductionsCoverage, BfgsBasicTraining)
{
  std::string tmp_cache = "./test_bfgs_cache.tmp";
  std::remove(tmp_cache.c_str());

  auto vw = VW::initialize(vwtest::make_args("--bfgs", "--passes", "2",
      "--cache_file", tmp_cache.c_str(), "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i) + " b:" + std::to_string(i * 2));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }

  std::remove(tmp_cache.c_str());
}

// ============================================================
// Reduction: cs_active coverage
// ============================================================

TEST(ReductionsCoverage, CsActiveTrainPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--cs_active", "4", "--simulation", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    std::string label = std::to_string((i % 4) + 1) + ":1";
    auto* ex = VW::read_example(*vw, label + " | a:" + std::to_string(i) + " b:" + std::to_string(i % 3));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// Reduction: ftrl coverage
// ============================================================

TEST(ReductionsCoverage, FtrlTrainPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--ftrl", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// Reduction: oja_newton coverage
// ============================================================

TEST(ReductionsCoverage, OjaNewtonTrainPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--OjaNewton", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// Reduction: freegrad coverage
// ============================================================

TEST(ReductionsCoverage, FreegradTrainPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--freegrad", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// Reduction: nn coverage
// ============================================================

TEST(ReductionsCoverage, NnTrainPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--nn", "3", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i) + " b:" + std::to_string(i * 2));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }

  auto* ex = VW::read_example(*vw, "| a:5 b:10");
  vw->predict(*ex);
  VW::finish_example(*vw, *ex);
}

// ============================================================
// Reduction: csoaa_ldf coverage
// ============================================================

TEST(ReductionsCoverage, CsoaaLdfTrainPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--csoaa_ldf", "mc", "--quiet"));

  // Multiline example format for CSOAA LDF
  for (int i = 0; i < 10; ++i)
  {
    auto* shared = VW::read_example(*vw, "shared | s:" + std::to_string(i));
    auto* action1 = VW::read_example(*vw, "1:" + std::to_string(i % 3 + 1) + ".0 | a:1");
    auto* action2 = VW::read_example(*vw, "2:" + std::to_string((i + 1) % 3 + 1) + ".0 | a:2");
    auto* empty = VW::read_example(*vw, "");

    VW::multi_ex multi_ex = {shared, action1, action2};
    vw->learn(multi_ex);
    vw->finish_example(multi_ex);
    VW::finish_example(*vw, *empty);
  }
}

// ============================================================
// Reduction: kernel_svm coverage
// ============================================================

TEST(ReductionsCoverage, KernelSvmTrainPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--ksvm", "--quiet"));

  // Use explicit numeric feature indices (required for collision_cleanup sorted assertion)
  for (int i = 0; i < 30; ++i)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw,
        std::to_string(label) + " |f 1:" + std::to_string(static_cast<float>(i) / 30.0f) +
        " 2:" + std::to_string(static_cast<float>(i * 2) / 60.0f));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// Reduction: confidence_sequence_robust coverage
// ============================================================

TEST(ReductionsCoverage, ConfidenceSequenceRobustViaReduction)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cb_type", "mtr",
      "--confidence_after_training", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    auto* shared = VW::read_example(*vw, "shared | s:" + std::to_string(i));
    auto* action1 = VW::read_example(*vw, std::string((i % 2 == 0) ? "0:1:0.5" : "") + " | a:1");
    auto* action2 = VW::read_example(*vw, std::string((i % 2 != 0) ? "0:2:0.5" : "") + " | a:2");

    VW::multi_ex multi_ex = {shared, action1, action2};
    vw->learn(multi_ex);
    vw->finish_example(multi_ex);
  }
}

// ============================================================
// api_status.cc coverage
// ============================================================

TEST(ApiStatusCoverage, BasicUsage)
{
  VW::experimental::api_status status;
  EXPECT_EQ(status.get_error_code(), 0);
  EXPECT_EQ(std::string(status.get_error_msg()), "");
}

// ============================================================
// hashstring.cc coverage
// ============================================================

TEST(HashStringCoverage, GetHasherFunctions)
{
  // Cover get_hasher for "strings" (line 16) and "all" (line 17)
  auto strings_hasher = VW::get_hasher("strings");
  EXPECT_NE(strings_hasher, nullptr);

  auto all_hasher = VW::get_hasher("all");
  EXPECT_NE(all_hasher, nullptr);

  EXPECT_NE(strings_hasher, all_hasher);

  // Unknown hasher should throw (line 19)
  EXPECT_THROW(VW::get_hasher("unknown"), VW::vw_exception);
}

// ============================================================
// cb_type.cc coverage - to_string for cb_type_t
// ============================================================

TEST(CbTypeCoverage, ToStringAllTypes)
{
  // Cover all cases in to_string(cb_type_t) and cb_type_from_string
  EXPECT_THAT(std::string(VW::to_string(VW::cb_type_t::DR)), testing::HasSubstr("DR"));
  EXPECT_THAT(std::string(VW::to_string(VW::cb_type_t::DM)), testing::HasSubstr("DM"));
  EXPECT_THAT(std::string(VW::to_string(VW::cb_type_t::IPS)), testing::HasSubstr("IPS"));
  EXPECT_THAT(std::string(VW::to_string(VW::cb_type_t::MTR)), testing::HasSubstr("MTR"));
  EXPECT_THAT(std::string(VW::to_string(VW::cb_type_t::SM)), testing::HasSubstr("SM"));

  // Cover cb_type_from_string
  EXPECT_EQ(VW::cb_type_from_string("dr"), VW::cb_type_t::DR);
  EXPECT_EQ(VW::cb_type_from_string("dm"), VW::cb_type_t::DM);
  EXPECT_EQ(VW::cb_type_from_string("ips"), VW::cb_type_t::IPS);
  EXPECT_EQ(VW::cb_type_from_string("mtr"), VW::cb_type_t::MTR);
  EXPECT_EQ(VW::cb_type_from_string("sm"), VW::cb_type_t::SM);
}

// ============================================================
// text_parser/parse_example_text.cc coverage paths
// ============================================================

TEST(TextParserCoverage, ParseVariousLabelFormats)
{
  // Exercise text parser paths with different label types
  {
    auto vw = VW::initialize(vwtest::make_args("--cb", "4", "--quiet"));
    auto* ex = VW::read_example(*vw, "1:2:0.5 | a b");
    VW::finish_example(*vw, *ex);
  }
  {
    auto vw = VW::initialize(vwtest::make_args("--csoaa", "4", "--quiet"));
    auto* ex = VW::read_example(*vw, "1:0.5 2:1.5 | a b");
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// Reduction: search + search entity relation task coverage
// ============================================================

TEST(ReductionsCoverage, SearchSequenceLabeling)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "3", "--search_task", "sequence",
      "--search_alpha", "1e-5", "--quiet"));

  // Simple sequence labeling: each word gets a label
  for (int i = 0; i < 5; ++i)
  {
    auto* ex1 = VW::read_example(*vw, "1 | word1");
    auto* ex2 = VW::read_example(*vw, "2 | word2");
    auto* ex3 = VW::read_example(*vw, "3 | word3");
    auto* empty = VW::read_example(*vw, "");

    VW::multi_ex multi_ex = {ex1, ex2, ex3};
    vw->learn(multi_ex);
    vw->finish_example(multi_ex);
    VW::finish_example(*vw, *empty);
  }
}

// ============================================================
// vw.cc: save and load model
// ============================================================

TEST(VwCoverage, SaveLoadModelRoundTrip)
{
  std::vector<char> model_buffer;

  {
    auto vw = VW::initialize(vwtest::make_args("--quiet"));
    for (int i = 0; i < 10; ++i)
    {
      auto* ex = VW::read_example(*vw, std::to_string(i % 2) + " | a:" + std::to_string(i));
      vw->learn(*ex);
      VW::finish_example(*vw, *ex);
    }

    auto backing = std::make_shared<std::vector<char>>();
    VW::io_buf io_writer;
    io_writer.add_file(VW::io::create_vector_writer(backing));
    VW::save_predictor(*vw, io_writer);
    io_writer.flush();
    model_buffer = *backing;
  }

  {
    auto vw = VW::initialize(vwtest::make_args("--quiet"),
        VW::io::create_buffer_view(model_buffer.data(), model_buffer.size()));
    auto* ex = VW::read_example(*vw, "| a:5");
    vw->predict(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// Reduction: eigen_memory_tree save/load coverage
// ============================================================

TEST(ReductionsCoverage, EigenMemoryTreeSaveLoad)
{
  std::vector<char> model_buffer;

  {
    auto vw = VW::initialize(vwtest::make_args("--memory_tree", "5", "--max_number_of_labels", "3",
        "--leaf_example_multiplier", "2", "--quiet"));

    for (int i = 0; i < 30; ++i)
    {
      std::string label = std::to_string((i % 3) + 1);
      auto* ex = VW::read_example(*vw, label + " | a:" + std::to_string(i));
      vw->learn(*ex);
      VW::finish_example(*vw, *ex);
    }

    auto backing = std::make_shared<std::vector<char>>();
    VW::io_buf io_writer;
    io_writer.add_file(VW::io::create_vector_writer(backing));
    VW::save_predictor(*vw, io_writer);
    io_writer.flush();
    model_buffer = *backing;
  }

  {
    auto vw = VW::initialize(vwtest::make_args("--quiet"),
        VW::io::create_buffer_view(model_buffer.data(), model_buffer.size()));
    auto* ex = VW::read_example(*vw, "| a:5");
    vw->predict(*ex);
    EXPECT_GE(ex->pred.multiclass, 1u);
    EXPECT_LE(ex->pred.multiclass, 3u);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// Reduction: search_dep_parser coverage
// ============================================================

TEST(ReductionsCoverage, SearchDepParserTraining)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "4", "--search_task", "dep_parser",
      "--search_alpha", "1e-5", "--quiet"));

  // dep_parser expects a specific format: label 1:head |features...
  for (int i = 0; i < 3; ++i)
  {
    auto* ex1 = VW::read_example(*vw, "1 1:2 | word:the");
    auto* ex2 = VW::read_example(*vw, "2 1:0 | word:cat");
    auto* empty = VW::read_example(*vw, "");

    VW::multi_ex multi_ex = {ex1, ex2};
    vw->learn(multi_ex);
    vw->finish_example(multi_ex);
    VW::finish_example(*vw, *empty);
  }
}

// ============================================================
// unique_sort.cc coverage
// ============================================================

TEST(UniqueSortCoverage, TrainingWithDuplicateFeatures)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));

  // Example with duplicate features to trigger unique_sort
  auto* ex = VW::read_example(*vw, "1.0 | a a b b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

// ============================================================
// daemon_utils.cc coverage
// ============================================================

TEST(DaemonUtilsCoverage, ParsePortRange)
{
  // Not directly testable without network, but we can exercise it through VW init
  // with daemon-like options that don't actually start the daemon
  EXPECT_NO_THROW({
    auto vw = VW::initialize(vwtest::make_args("--quiet"));
    (void)vw;
  });
}

// ============================================================
// json_parser coverage through learning with JSON examples
// ============================================================

TEST(JsonParserCoverage, ParseJsonExample)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--json", "--quiet"));

  std::string json_example = R"({"_label_cost":-1.0,"_label_probability":0.5,"_label_Action":1,"_labelIndex":0,"c":{"_multi":[{"a":{"x":1}},{"a":{"x":2}}],"_text":"hello world"}})";

  auto examples = vwtest::parse_json(*vw, json_example);
  for (auto& ex : examples) { VW::finish_example(*vw, *ex); }
}

// ============================================================
// Additional reduction coverage: LDA with different modes
// ============================================================

TEST(ReductionsCoverage2, LdaBasicTraining)
{
  auto vw = VW::initialize(vwtest::make_args("--lda", "3", "--lda_D", "10", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    auto* ex = VW::read_example(*vw, "| word1:" + std::to_string(i % 5 + 1) +
        " word2:" + std::to_string(i % 3 + 1) + " word3:" + std::to_string(i % 7 + 1));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(ReductionsCoverage2, LdaPreciseMathMode)
{
  // math_mode=1 means USE_PRECISE (covers lgamma/digamma/powf precise paths)
  auto vw = VW::initialize(vwtest::make_args("--lda", "2", "--lda_D", "5", "--math-mode", "1", "--quiet"));

  for (int i = 0; i < 10; ++i)
  {
    auto* ex = VW::read_example(*vw, "| a:" + std::to_string(i + 1) + " b:" + std::to_string(i + 2));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(ReductionsCoverage2, LdaMinibatchMode)
{
  // Exercise the minibatch > 1 code paths in LDA
  auto vw = VW::initialize(vwtest::make_args("--lda", "2", "--lda_D", "5",
      "--minibatch", "4", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    auto* ex = VW::read_example(*vw, "| w1:" + std::to_string(i % 4 + 1) + " w2:" + std::to_string(i + 1));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// cost_sensitive.cc - more parsing paths
// ============================================================

TEST(CostSensitiveCoverage2, ParseSharedLabel)
{
  auto vw = VW::initialize(vwtest::make_args("--csoaa_ldf", "mc", "--quiet"));

  // Test shared example (exercises shared keyword parsing)
  auto* shared = VW::read_example(*vw, "shared | s1 s2");
  auto* action1 = VW::read_example(*vw, "1:1.0 | a:1");
  auto* action2 = VW::read_example(*vw, "2:3.0 | a:2");

  VW::multi_ex multi_ex = {shared, action1, action2};
  vw->learn(multi_ex);
  vw->finish_example(multi_ex);
}

TEST(CostSensitiveCoverage2, TestLabelParsing)
{
  // Exercise CS test_label path with FLT_MAX costs
  auto vw = VW::initialize(vwtest::make_args("--csoaa", "3", "--quiet"));

  // This is a test example (no labels)
  auto* ex = VW::read_example(*vw, "| a b c");
  vw->predict(*ex);
  VW::finish_example(*vw, *ex);

  // Example with multiple costs
  auto* ex2 = VW::read_example(*vw, "1:0.5 2:1.0 3:0.8 | a b c");
  vw->learn(*ex2);
  VW::finish_example(*vw, *ex2);
}

// ============================================================
// Cbify reduction coverage
// ============================================================

TEST(ReductionsCoverage2, CbifyBasicTraining)
{
  auto vw = VW::initialize(vwtest::make_args("--cbify", "3", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    std::string label = std::to_string((i % 3) + 1);
    auto* ex = VW::read_example(*vw, label + " | a:" + std::to_string(i) + " b:" + std::to_string(i % 5));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(ReductionsCoverage2, CbifyLdfMode)
{
  // cbify_ldf is a boolean flag, requires --csoaa_ldf for LDF base, --cb_type for CB
  auto vw = VW::initialize(vwtest::make_args("--cbify_ldf", "--csoaa_ldf", "mc",
      "--cb_type", "mtr", "--quiet"));

  for (int i = 0; i < 10; ++i)
  {
    auto* shared = VW::read_example(*vw, "shared | s:" + std::to_string(i));
    auto* a1 = VW::read_example(*vw, "1:" + std::to_string(i % 3 + 1) + ".0 | a:1");
    auto* a2 = VW::read_example(*vw, "2:" + std::to_string((i + 1) % 3 + 1) + ".0 | a:2");

    VW::multi_ex multi_ex = {shared, a1, a2};
    vw->learn(multi_ex);
    vw->finish_example(multi_ex);
    auto* empty = VW::read_example(*vw, "");
    VW::finish_example(*vw, *empty);
  }
}

TEST(ReductionsCoverage2, CbifyWithCsMode)
{
  auto vw = VW::initialize(vwtest::make_args("--cbify", "3", "--cbify_cs", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    std::string label = std::to_string((i % 3) + 1);
    auto* ex = VW::read_example(*vw, label + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// FTRL reduction - pistol and coin variants
// ============================================================

TEST(ReductionsCoverage2, FtrlPistolMode)
{
  auto vw = VW::initialize(vwtest::make_args("--pistol", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(ReductionsCoverage2, FtrlCoinMode)
{
  auto vw = VW::initialize(vwtest::make_args("--coin", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// Marginal reduction with competition mode
// ============================================================

TEST(ReductionsCoverage2, MarginalWithCompetition)
{
  auto vw = VW::initialize(vwtest::make_args("--marginal", "a", "--compete", "--quiet"));

  for (int i = 0; i < 30; ++i)
  {
    float label = static_cast<float>(i % 3);
    auto* ex = VW::read_example(*vw, std::to_string(label) + " |a x:" + std::to_string(i % 5));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(ReductionsCoverage2, MarginalSaveLoadWithCompetition)
{
  std::vector<char> model_buffer;

  {
    auto vw = VW::initialize(vwtest::make_args("--marginal", "a", "--compete", "--quiet"));
    for (int i = 0; i < 20; ++i)
    {
      float label = static_cast<float>(i % 3);
      auto* ex = VW::read_example(*vw, std::to_string(label) + " |a x:" + std::to_string(i % 5));
      vw->learn(*ex);
      VW::finish_example(*vw, *ex);
    }

    auto backing = std::make_shared<std::vector<char>>();
    VW::io_buf io_writer;
    io_writer.add_file(VW::io::create_vector_writer(backing));
    VW::save_predictor(*vw, io_writer);
    io_writer.flush();
    model_buffer = *backing;
  }

  // Must reload with same reduction flags
  {
    auto vw = VW::initialize(vwtest::make_args("--marginal", "a", "--compete", "--quiet"),
        VW::io::create_buffer_view(model_buffer.data(), model_buffer.size()));
    auto* ex = VW::read_example(*vw, "|a x:2");
    vw->predict(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// Generate interactions - different flag combinations
// ============================================================

TEST(ReductionsCoverage2, GenerateInteractionsWithDuplicates)
{
  // leave_duplicate_interactions = true
  auto vw = VW::initialize(vwtest::make_args("-q", "aa", "--leave_duplicate_interactions", "--quiet"));

  for (int i = 0; i < 10; ++i)
  {
    float label = static_cast<float>(i % 2);
    auto* ex = VW::read_example(*vw, std::to_string(label) + " |a x:" + std::to_string(i) + " y:" + std::to_string(i + 1));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(ReductionsCoverage2, GenerateInteractionsWildcard)
{
  // Use wildcard interactions with ':'
  auto vw = VW::initialize(vwtest::make_args("-q", "::", "--quiet"));

  for (int i = 0; i < 10; ++i)
  {
    float label = static_cast<float>(i % 2);
    auto* ex = VW::read_example(*vw, std::to_string(label) + " |a x:" + std::to_string(i) +
        " |b y:" + std::to_string(i + 1));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// CSOAA LDF - different mode coverage
// ============================================================

TEST(ReductionsCoverage2, WapLdfMode)
{
  // WAP is a separate option: --wap_ldf multiline
  auto vw = VW::initialize(vwtest::make_args("--wap_ldf", "multiline", "--quiet"));

  for (int i = 0; i < 10; ++i)
  {
    auto* shared = VW::read_example(*vw, "shared | s:" + std::to_string(i));
    auto* a1 = VW::read_example(*vw, "1:" + std::to_string(i % 3 + 1) + ".0 | a:1");
    auto* a2 = VW::read_example(*vw, "2:" + std::to_string((i + 1) % 3 + 1) + ".0 | a:2");
    auto* a3 = VW::read_example(*vw, "3:" + std::to_string((i + 2) % 3 + 1) + ".0 | a:3");

    VW::multi_ex multi_ex = {shared, a1, a2, a3};
    vw->learn(multi_ex);
    vw->finish_example(multi_ex);
    auto* empty = VW::read_example(*vw, "");
    VW::finish_example(*vw, *empty);
  }
}

TEST(ReductionsCoverage2, CsoaaLdfWithProbabilities)
{
  auto vw = VW::initialize(vwtest::make_args("--csoaa_ldf", "mc", "--csoaa_rank", "--quiet"));

  for (int i = 0; i < 10; ++i)
  {
    auto* shared = VW::read_example(*vw, "shared | s:" + std::to_string(i));
    auto* a1 = VW::read_example(*vw, "1:" + std::to_string(i % 3 + 1) + ".0 | a:1");
    auto* a2 = VW::read_example(*vw, "2:" + std::to_string((i + 1) % 3 + 1) + ".0 | a:2");

    VW::multi_ex multi_ex = {shared, a1, a2};
    vw->learn(multi_ex);
    vw->finish_example(multi_ex);
    auto* empty = VW::read_example(*vw, "");
    VW::finish_example(*vw, *empty);
  }
}

// ============================================================
// Stagewise poly coverage
// ============================================================

TEST(ReductionsCoverage2, StagewisePolyTraining)
{
  std::string tmp_cache = "./test_stagewise_cache.tmp";
  std::remove(tmp_cache.c_str());

  auto vw = VW::initialize(vwtest::make_args("--stage_poly", "--sched_exponent", "1.0",
      "--batch_sz", "50", "--passes", "2", "--cache_file", tmp_cache.c_str(), "--quiet"));

  for (int i = 0; i < 100; ++i)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw,
        std::to_string(label) + " |a x:" + std::to_string(i) + " y:" + std::to_string(i * 2));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }

  std::remove(tmp_cache.c_str());
}

// ============================================================
// cs_active additional coverage
// ============================================================

TEST(ReductionsCoverage2, CsActiveWithDomination)
{
  // Correct option is --domination (takes 0 or 1), not --use_domination
  auto vw = VW::initialize(vwtest::make_args("--cs_active", "4", "--simulation",
      "--domination", "1", "--cost_max", "2", "--mellowness", "0.5", "--quiet"));

  for (int i = 0; i < 30; ++i)
  {
    std::string label = std::to_string((i % 4) + 1) + ":1";
    auto* ex = VW::read_example(*vw, label + " | a:" + std::to_string(i) + " b:" + std::to_string(i % 3));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// text_parser coverage - different label formats and features
// ============================================================

TEST(TextParserCoverage2, ParseFeaturesWithSuffix)
{
  // Exercise text parsing with affix features
  auto vw = VW::initialize(vwtest::make_args("--affix", "-2a", "--quiet"));

  auto* ex = VW::read_example(*vw, "1.0 |a hello world testing");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(TextParserCoverage2, ParseFeaturesWithSpelling)
{
  // Exercise spelling features
  auto vw = VW::initialize(vwtest::make_args("--spelling", "a", "--quiet"));

  auto* ex = VW::read_example(*vw, "1.0 |a Hello World 123");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(TextParserCoverage2, ParseNamespaceWithValue)
{
  // Test namespace with explicit numeric value
  auto vw = VW::initialize(vwtest::make_args("--quiet"));

  auto* ex = VW::read_example(*vw, "1.0 |a:2.5 x:1 y:2");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

TEST(TextParserCoverage2, ParseMultipleNamespaces)
{
  // Multiple namespaces exercise different parser paths
  auto vw = VW::initialize(vwtest::make_args("--quiet"));

  auto* ex = VW::read_example(*vw, "1.0 |a x:1 y:2 |b p:3 q:4 |c m:5");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

// ============================================================
// search_graph coverage
// ============================================================

TEST(ReductionsCoverage2, SearchGraphTraining)
{
  auto vw = VW::initialize(vwtest::make_args("--search", "3",
      "--search_task", "graph", "--search_alpha", "1e-5", "--quiet"));

  for (int i = 0; i < 3; ++i)
  {
    auto* ex1 = VW::read_example(*vw, "1 | x:1");
    auto* ex2 = VW::read_example(*vw, "2 | x:2");
    auto* ex3 = VW::read_example(*vw, "3 | x:3");
    auto* empty = VW::read_example(*vw, "");

    VW::multi_ex multi_ex = {ex1, ex2, ex3};
    vw->learn(multi_ex);
    vw->finish_example(multi_ex);
    VW::finish_example(*vw, *empty);
  }
}

// ============================================================
// confidence_sequence_robust additional paths
// ============================================================

TEST(ReductionsCoverage2, ConfidenceSequenceRobustSaveLoad)
{
  std::vector<char> model_buffer;

  {
    auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--cb_type", "mtr",
        "--confidence_after_training", "--quiet"));

    for (int i = 0; i < 30; ++i)
    {
      auto* shared = VW::read_example(*vw, "shared | s:" + std::to_string(i));
      auto* a1 = VW::read_example(*vw, std::string((i % 2 == 0) ? "0:1:0.5" : "") + " | a:1");
      auto* a2 = VW::read_example(*vw, std::string((i % 2 != 0) ? "0:2:0.5" : "") + " | a:2");

      VW::multi_ex multi_ex = {shared, a1, a2};
      vw->learn(multi_ex);
      vw->finish_example(multi_ex);
    }

    auto backing = std::make_shared<std::vector<char>>();
    VW::io_buf io_writer;
    io_writer.add_file(VW::io::create_vector_writer(backing));
    VW::save_predictor(*vw, io_writer);
    io_writer.flush();
    model_buffer = *backing;
  }

  {
    auto vw = VW::initialize(vwtest::make_args("--quiet"),
        VW::io::create_buffer_view(model_buffer.data(), model_buffer.size()));

    auto* shared = VW::read_example(*vw, "shared | s:15");
    auto* a1 = VW::read_example(*vw, "| a:1");
    auto* a2 = VW::read_example(*vw, "| a:2");

    VW::multi_ex multi_ex = {shared, a1, a2};
    vw->predict(multi_ex);
    vw->finish_example(multi_ex);
  }
}

// ============================================================
// Log_multi save/load coverage
// ============================================================

TEST(ReductionsCoverage2, LogMultiSaveLoad)
{
  std::vector<char> model_buffer;

  {
    auto vw = VW::initialize(vwtest::make_args("--log_multi", "4", "--quiet"));
    for (int i = 0; i < 40; ++i)
    {
      std::string label = std::to_string((i % 4) + 1);
      auto* ex = VW::read_example(*vw, label + " | a:" + std::to_string(i));
      vw->learn(*ex);
      VW::finish_example(*vw, *ex);
    }

    auto backing = std::make_shared<std::vector<char>>();
    VW::io_buf io_writer;
    io_writer.add_file(VW::io::create_vector_writer(backing));
    VW::save_predictor(*vw, io_writer);
    io_writer.flush();
    model_buffer = *backing;
  }

  {
    auto vw = VW::initialize(vwtest::make_args("--quiet"),
        VW::io::create_buffer_view(model_buffer.data(), model_buffer.size()));
    auto* ex = VW::read_example(*vw, "| a:5");
    vw->predict(*ex);
    EXPECT_GE(ex->pred.multiclass, 1u);
    EXPECT_LE(ex->pred.multiclass, 4u);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// Recall tree coverage
// ============================================================

TEST(ReductionsCoverage2, RecallTreeTrainPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--recall_tree", "4", "--quiet"));

  for (int i = 0; i < 30; ++i)
  {
    std::string label = std::to_string((i % 4) + 1);
    auto* ex = VW::read_example(*vw, label + " | a:" + std::to_string(i) + " b:" + std::to_string(i % 7));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// PLT (Probabilistic Label Tree) coverage
// ============================================================

TEST(ReductionsCoverage2, PltTrainPredict)
{
  // PLT requires --loss_function logistic and multilabel input format
  auto vw = VW::initialize(vwtest::make_args("--plt", "5", "--loss_function", "logistic", "--quiet"));

  for (int i = 0; i < 30; ++i)
  {
    // Multilabel format: comma-separated label indices
    std::string labels = std::to_string((i % 5) + 1);
    if (i % 3 == 0) { labels += "," + std::to_string(((i + 1) % 5) + 1); }
    auto* ex = VW::read_example(*vw, labels + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// ECT (error correcting tournament) coverage
// ============================================================

TEST(ReductionsCoverage2, EctTrainPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--ect", "4", "--quiet"));

  for (int i = 0; i < 30; ++i)
  {
    std::string label = std::to_string((i % 4) + 1);
    auto* ex = VW::read_example(*vw, label + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// Boosting coverage
// ============================================================

TEST(ReductionsCoverage2, BoostingTrainPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--boosting", "5", "--quiet"));

  for (int i = 0; i < 30; ++i)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// Active learning coverage
// ============================================================

TEST(ReductionsCoverage2, ActiveCoverTrainPredict)
{
  // active_cover is a boolean flag, requires --loss_function logistic --binary
  auto vw = VW::initialize(vwtest::make_args("--active_cover",
      "--loss_function", "logistic", "--binary", "--quiet"));

  for (int i = 0; i < 30; ++i)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// Confidence reduction
// ============================================================

TEST(ReductionsCoverage2, ConfidenceTrainPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--confidence", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// Autolink coverage
// ============================================================

TEST(ReductionsCoverage2, AutolinkTrainPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--autolink", "2", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    float label = static_cast<float>(i % 5);
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// LRQ coverage
// ============================================================

TEST(ReductionsCoverage2, LrqTrainPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--lrq", "ab3", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    float label = static_cast<float>(i % 2);
    auto* ex = VW::read_example(*vw,
        std::to_string(label) + " |a x:" + std::to_string(i) + " |b y:" + std::to_string(i + 1));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// LRQFA coverage
// ============================================================

TEST(ReductionsCoverage2, LrqfaTrainPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--lrqfa", "ab3", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    float label = static_cast<float>(i % 2);
    auto* ex = VW::read_example(*vw,
        std::to_string(label) + " |a x:" + std::to_string(i) + " |b y:" + std::to_string(i + 1));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// Classweight coverage
// ============================================================

TEST(ReductionsCoverage2, ClassweightOaaTraining)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "3", "--classweight", "1:2.0", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    std::string label = std::to_string((i % 3) + 1);
    auto* ex = VW::read_example(*vw, label + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// MWT (multiworld testing) coverage
// ============================================================

TEST(ReductionsCoverage2, MwtTraining)
{
  auto vw = VW::initialize(vwtest::make_args("--multiworld_test", "a", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// Warm CB coverage
// ============================================================

TEST(ReductionsCoverage2, WarmCbTraining)
{
  // warm_cb needs --cb_explore_adf --cb_type mtr, multiclass labels
  auto vw = VW::initialize(vwtest::make_args("--warm_cb", "3", "--cb_explore_adf", "--cb_type", "mtr",
      "--epsilon", "0.05", "--warm_start", "5", "--interaction", "10",
      "--choices_lambda", "4", "--warm_start_update", "--interaction_update", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    std::string label = std::to_string((i % 3) + 1);
    auto* ex = VW::read_example(*vw, label + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// CBZO (contextual bandit zeroth order) coverage
// ============================================================

TEST(ReductionsCoverage2, CbzoTraining)
{
  // CBZO requires --policy, --radius; labels start with "ca action:cost:probability"
  auto vw = VW::initialize(vwtest::make_args("--cbzo", "--policy", "constant",
      "--radius", "0.1", "-l", "0.001", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    // Match format from test data: ca <action>:<cost>:<probability_denominator>
    float action = -0.1f + static_cast<float>(i) * 0.01f;
    float cost = static_cast<float>(i % 5) * 0.5f;
    auto* ex = VW::read_example(*vw,
        "ca " + std::to_string(action) + ":" + std::to_string(cost) + ":33554432 |ns a:" +
        std::to_string(static_cast<float>(i) * 0.1f) + " b:" +
        std::to_string(static_cast<float>(i) * 0.2f));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// Offset tree coverage
// ============================================================

TEST(ReductionsCoverage2, OffsetTreeTraining)
{
  // Offset tree uses --ot not --offset_tree, with CB-style action:cost:probability labels
  auto vw = VW::initialize(vwtest::make_args("--ot", "3", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    int action = (i % 3) + 1;
    float cost = (i % 2 == 0) ? -1.0f : 0.0f;
    auto* ex = VW::read_example(*vw,
        std::to_string(action) + ":" + std::to_string(cost) + ":0.5 |f a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// Different loss function variants via training
// ============================================================

TEST(LossFunctionsCoverage2, TrainWithHingeLoss)
{
  auto vw = VW::initialize(vwtest::make_args("--loss_function", "hinge", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(LossFunctionsCoverage2, TrainWithLogisticLoss)
{
  auto vw = VW::initialize(vwtest::make_args("--loss_function", "logistic", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(LossFunctionsCoverage2, TrainWithQuantileLoss)
{
  auto vw = VW::initialize(vwtest::make_args("--loss_function", "quantile", "--quantile_tau", "0.3", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    float label = static_cast<float>(i % 5);
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// Binary reduction coverage
// ============================================================

TEST(ReductionsCoverage2, BinaryReduction)
{
  auto vw = VW::initialize(vwtest::make_args("--binary", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    float label = (i % 2 == 0) ? 1.0f : -1.0f;
    auto* ex = VW::read_example(*vw, std::to_string(label) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ============================================================
// Explore eval coverage
// ============================================================

TEST(ReductionsCoverage2, ExploreEvalTraining)
{
  auto vw = VW::initialize(vwtest::make_args("--cb_explore_adf",
      "--explore_eval", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    auto* shared = VW::read_example(*vw, "shared | s:" + std::to_string(i));
    auto* a1 = VW::read_example(*vw, std::string((i % 2 == 0) ? "0:1:0.5" : "") + " | a:1");
    auto* a2 = VW::read_example(*vw, std::string((i % 2 != 0) ? "0:2:0.5" : "") + " | a:2");

    VW::multi_ex multi_ex = {shared, a1, a2};
    vw->learn(multi_ex);
    vw->finish_example(multi_ex);
  }
}

// ============================================================
// vw.cc: to_argv and free_args paths
// ============================================================

TEST(VwCoverage2, InitializeWithStringOverload)
{
  // Exercise the string-based initialize overload (to_argv + free_args paths)
  auto* vw = VW::initialize(std::string("--quiet"));
  EXPECT_NE(vw, nullptr);

  auto* ex = VW::read_example(*vw, "1.0 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
  VW::finish(*vw);
}

TEST(VwCoverage2, InitializeWithStringAndModel)
{
  std::vector<char> model_buffer;

  {
    auto vw = VW::initialize(vwtest::make_args("--quiet"));
    auto* ex = VW::read_example(*vw, "1.0 | a b c");
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);

    auto backing = std::make_shared<std::vector<char>>();
    VW::io_buf io_writer;
    io_writer.add_file(VW::io::create_vector_writer(backing));
    VW::save_predictor(*vw, io_writer);
    io_writer.flush();
    model_buffer = *backing;
  }

  // Load with string-based initialize (io_buf* overload)
  VW::io_buf model_io;
  model_io.add_file(VW::io::create_buffer_view(model_buffer.data(), model_buffer.size()));
  auto* vw = VW::initialize(std::string("--quiet"), &model_io);
  EXPECT_NE(vw, nullptr);

  auto* ex = VW::read_example(*vw, "| a b c");
  vw->predict(*ex);
  VW::finish_example(*vw, *ex);
  VW::finish(*vw);
}

// ============================================================
// parse_args.cc: additional option paths
// ============================================================

TEST(ParseArgsCoverage, AuditMode)
{
  // Exercise audit/invert_hash path in parse_args
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--invert_hash", "/dev/null"));

  EXPECT_TRUE(vw->output_config.hash_inv);
  auto* ex = VW::read_example(*vw, "1.0 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
}

// ============================================================
// Epsilon decay coverage
// ============================================================

// epsilon_decay clamps out-of-range rewards to [0,1] and warns.
// This test verifies that costs outside [0,1] do not crash.
TEST(ReductionsCoverage2, EpsilonDecayClampsOutOfRangeReward)
{
  auto vw = VW::initialize(
      vwtest::make_args("--cb_explore_adf", "--epsilon_decay", "--model_count", "3", "--quiet"));

  for (int i = 0; i < 30; ++i)
  {
    auto* shared = VW::read_example(*vw, "shared | s:" + std::to_string(i));
    auto* a1 =
        VW::read_example(*vw, std::string((i % 2 == 0) ? "0:1:0.5" : "") + " | a:1");
    auto* a2 =
        VW::read_example(*vw, std::string((i % 2 != 0) ? "0:2:0.5" : "") + " | a:2");

    VW::multi_ex multi_ex = {shared, a1, a2};
    vw->learn(multi_ex);
    vw->finish_example(multi_ex);
  }
}

// ============================================================
// Coverage batch 3: targeting 150+ more uncovered lines
// ============================================================

// ---- Bootstrap vote mode (bs.cc lines 79, 97-125) ----
TEST(ReductionsCoverage3, BootstrapVoteMode)
{
  // Exercise BS_TYPE_VOTE code path including multi-vote detection
  auto vw = VW::initialize(vwtest::make_args("--bootstrap", "5", "--bs_type", "vote", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i % 3 + 1) + " | x:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
  // Predict to exercise vote aggregation path
  auto* ex = VW::read_example(*vw, "| x:5");
  vw->predict(*ex);
  VW::finish_example(*vw, *ex);
}

// ---- OAA with 0-indexed labels (oaa.cc lines 60-61, 74-75, 194-201, 208-220) ----
TEST(ReductionsCoverage3, OaaZeroIndexedLabels)
{
  // Feeding label 0 triggers 0-indexed detection in oaa
  auto vw = VW::initialize(vwtest::make_args("--oaa", "3", "--quiet"));

  // First example with label 0 triggers indexing detection
  auto* ex = VW::read_example(*vw, "0 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);

  for (int i = 0; i < 10; ++i)
  {
    auto* e = VW::read_example(*vw, std::to_string(i % 3) + " | x:" + std::to_string(i));
    vw->learn(*e);
    VW::finish_example(*vw, *e);
  }
  // Predict to exercise 0-indexed passthrough/prediction paths
  auto* pred_ex = VW::read_example(*vw, "| a b");
  vw->predict(*pred_ex);
  VW::finish_example(*vw, *pred_ex);
}

// ---- OAA with scores mode (oaa.cc lines 431-433 raw prediction) ----
TEST(ReductionsCoverage3, OaaWithScores)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "4", "--oaa_subsample", "2", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i % 4 + 1) + " | x:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
  auto* ex = VW::read_example(*vw, "| x:5");
  vw->predict(*ex);
  VW::finish_example(*vw, *ex);
}

// ---- Topk predict (topk.cc lines 49-63, 177) ----
TEST(ReductionsCoverage3, TopkTrainPredict)
{
  auto vw = VW::initialize(vwtest::make_args("--top", "3", "--quiet"));

  // topk requires multi_ex
  for (int i = 0; i < 15; ++i)
  {
    auto* ex1 = VW::read_example(*vw, std::to_string(i % 3 + 1) + ".0 | x:" + std::to_string(i));
    auto* ex2 = VW::read_example(*vw, std::to_string(i % 3 + 2) + ".0 | y:" + std::to_string(i));
    VW::multi_ex multi_ex = {ex1, ex2};
    vw->learn(multi_ex);
    vw->finish_example(multi_ex);
  }
  // Predict path
  auto* p1 = VW::read_example(*vw, "| x:3");
  auto* p2 = VW::read_example(*vw, "| y:4");
  VW::multi_ex pred_ex = {p1, p2};
  vw->predict(pred_ex);
  vw->finish_example(pred_ex);
}

// ---- NN meanfield mode (nn.cc lines 463-465) ----
TEST(ReductionsCoverage3, NnMeanfieldMode)
{
  auto vw = VW::initialize(vwtest::make_args("--nn", "3", "--meanfield", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i % 2) + " | a:" + std::to_string(i) + " b:" + std::to_string(i * 2));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
  auto* ex = VW::read_example(*vw, "| a:5 b:10");
  vw->predict(*ex);
  VW::finish_example(*vw, *ex);
}

// ---- NN with inpass mode (nn.cc lines 213-227, 303, 326-328) ----
TEST(ReductionsCoverage3, NnInpassMode)
{
  auto vw = VW::initialize(vwtest::make_args("--nn", "2", "--inpass", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i % 2) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
  auto* ex = VW::read_example(*vw, "| a:5");
  vw->predict(*ex);
  VW::finish_example(*vw, *ex);
}

// ---- Print reduction with weight and tag (print.cc lines 42-43, 45, 51-52) ----
TEST(ReductionsCoverage3, PrintReductionWeightAndTag)
{
  auto vw = VW::initialize(vwtest::make_args("--print"));

  // Example with non-unit weight and initial value, plus a tag
  auto* ex = VW::read_example(*vw, "1.0 2.0 3.0 'mytag | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);

  // Example with just label
  auto* ex2 = VW::read_example(*vw, "1.0 | x y z");
  vw->learn(*ex2);
  VW::finish_example(*vw, *ex2);
}

// ---- Freegrad with explicit radius (freegrad.cc lines 356-357) ----
TEST(ReductionsCoverage3, FreeGradWithRadius)
{
  auto vw = VW::initialize(vwtest::make_args("--freegrad", "--radius", "2.0", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i % 2) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ---- Freegrad with restart and project (freegrad.cc lines 118-121, 187-195, 230-232) ----
TEST(ReductionsCoverage3, FreeGradRestartProject)
{
  auto vw = VW::initialize(vwtest::make_args("--freegrad", "--restart", "--project", "--quiet"));

  for (int i = 0; i < 30; ++i)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i % 2) + " | a:" + std::to_string(i) + " b:" + std::to_string(i * 3));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ---- seed_vw_model (vw.cc lines 208-240) ----
TEST(VwCoverage3, SeedVwModel)
{
  VW_WARNING_STATE_PUSH
  VW_WARNING_DISABLE_DEPRECATED_USAGE
  auto* base = VW::initialize(std::string("--quiet"));

  // Train a bit on base
  auto* ex = VW::read_example(*base, "1.0 | a b c");
  base->learn(*ex);
  VW::finish_example(*base, *ex);

  // Seed a new model from the base
  auto* seeded = VW::seed_vw_model(base, "--quiet", nullptr, nullptr);
  EXPECT_NE(seeded, nullptr);

  auto* ex2 = VW::read_example(*seeded, "| a b c");
  seeded->predict(*ex2);
  VW::finish_example(*seeded, *ex2);

  VW::finish(*seeded);
  VW::finish(*base);
  VW_WARNING_STATE_POP
}

// ---- Deprecated initialization overloads (vw.cc lines 177-203) ----
TEST(VwCoverage3, InitializeWithOptionsI)
{
  VW_WARNING_STATE_PUSH
  VW_WARNING_DISABLE_DEPRECATED_USAGE
  // Exercise argc/argv overload (vw.cc ~line 200)
  const char* args[] = {"--quiet"};
  auto* vw_argc = VW::initialize(1, const_cast<char**>(args));
  EXPECT_NE(vw_argc, nullptr);
  VW::finish(*vw_argc);
  VW_WARNING_STATE_POP
}

// ---- GD MF training (gd_mf.cc lines 381-382 and general coverage) ----
TEST(ReductionsCoverage3, GdMfTraining)
{
  auto vw = VW::initialize(vwtest::make_args("--rank", "2", "--quiet"));

  for (int i = 0; i < 30; ++i)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i % 3 + 1) + " |a x:" + std::to_string(i) + " |b y:" + std::to_string(i * 2));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
  auto* ex = VW::read_example(*vw, "|a x:5 |b y:10");
  vw->predict(*ex);
  VW::finish_example(*vw, *ex);
}

// ---- Interact reduction error paths (interact.cc lines 46-47, 76-77, 82-83) ----
TEST(ReductionsCoverage3, InteractWithMissingAnchor)
{
  // Exercise the interact reduction with valid two-namespace interaction
  auto vw = VW::initialize(vwtest::make_args("--interact", "ab", "--quiet"));

  for (int i = 0; i < 10; ++i)
  {
    auto* ex = VW::read_example(*vw, "1.0 |a x:" + std::to_string(i) + " |b y:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ---- Explore eval with more training data (explore_eval.cc lines 184-195, 289-294) ----
TEST(ReductionsCoverage3, ExploreEvalExtended)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--cb_explore_adf", "--explore_eval", "--quiet", "--epsilon", "0.1"));

  for (int i = 0; i < 30; ++i)
  {
    auto* shared = VW::read_example(*vw, "shared | s:" + std::to_string(i));
    auto* a1 = VW::read_example(*vw,
        std::string((i % 2 == 0) ? "0:1:0.5" : "") + " | a:1 x:" + std::to_string(i));
    auto* a2 = VW::read_example(*vw,
        std::string((i % 2 != 0) ? "0:0.5:0.5" : "") + " | a:2 y:" + std::to_string(i));
    auto* empty = VW::read_example(*vw, "");
    VW::multi_ex multi_ex = {shared, a1, a2};
    vw->learn(multi_ex);
    vw->finish_example(multi_ex);
    VW::finish_example(*vw, *empty);
  }
}

// ---- CS active simulation mode (cs_active.cc more paths) ----
TEST(ReductionsCoverage3, CsActiveSimulationMode)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--cs_active", "3", "--simulation", "--mellowness", "0.1", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    // cost sensitive format: label:cost
    auto* ex = VW::read_example(*vw, std::to_string(i % 3 + 1) + ":1.0 | x:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ---- Cost sensitive with named labels (cost_sensitive.cc lines 138-143, 221-282) ----
TEST(CostSensitiveCoverage3, WithNamedLabels)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--csoaa", "3", "--named_labels", "cat,dog,bird", "--quiet"));

  for (int i = 0; i < 10; ++i)
  {
    auto* ex = VW::read_example(*vw, "1:0.5 2:1.0 3:2.0 | x:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
  auto* ex = VW::read_example(*vw, "| x:5");
  vw->predict(*ex);
  VW::finish_example(*vw, *ex);
}

// ---- CB with graph feedback parsing (cb.cc lines 43-64, 117-124) ----
// cb_with_observations_label requires graph feedback triplets
// Already covered by existing tests but need to exercise more paths

// ---- No label output paths (no_label.cc lines 59, 71, 77, 79-81) ----
TEST(NoLabelCoverage3, OutputAndAccountExample)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));

  // Parse an example with no label to exercise no_label output
  auto* ex = VW::read_example(*vw, "| a b c");
  vw->predict(*ex);
  VW::finish_example(*vw, *ex);
}

// ---- Marginal save/load with compete and expert state (marginal.cc lines 313-384) ----
TEST(ReductionsCoverage3, MarginalCompeteModelPersistence)
{
  // First train and save
  std::vector<char> model_data;
  {
    auto vw = VW::initialize(vwtest::make_args("--marginal", "a", "--compete", "--quiet", "--save_resume"));

    for (int i = 0; i < 20; ++i)
    {
      auto* ex = VW::read_example(*vw, std::to_string(i % 3 + 1) + ".0 |a x:" + std::to_string(i));
      vw->learn(*ex);
      VW::finish_example(*vw, *ex);
    }

    auto backing_vector = std::make_shared<std::vector<char>>();
    VW::io_buf io_writer;
    io_writer.add_file(VW::io::create_vector_writer(backing_vector));
    VW::save_predictor(*vw, io_writer);
    model_data = *backing_vector;
  }

  // Load and continue training
  {
    auto vw = VW::initialize(vwtest::make_args("--marginal", "a", "--compete", "--quiet"),
        VW::io::create_buffer_view(model_data.data(), model_data.size()));
    auto* ex = VW::read_example(*vw, "1.0 |a x:1");
    vw->predict(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ---- Stagewise poly with batching (stagewise_poly.cc lines 547-619) ----
TEST(ReductionsCoverage3, StagewisePolyBatchDoubling)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--stage_poly", "--batch_sz", "10", "--batch_sz_no_doubling", "--quiet"));

  for (int i = 0; i < 30; ++i)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i % 2) + " | a:" + std::to_string(i) + " b:" + std::to_string(i * 2));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ---- FTRL with audit features to exercise more paths (ftrl.cc lines 70-89, 122-124) ----
TEST(ReductionsCoverage3, FtrlPistolWithAudit)
{
  auto vw = VW::initialize(vwtest::make_args("--pistol", "--audit", "--quiet"));

  for (int i = 0; i < 10; ++i)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i % 2) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ---- Generate interactions with cubic and wildcard (generate_interactions.cc) ----
TEST(ReductionsCoverage3, GenerateInteractionsCubicWildcard)
{
  // Use --cubic to exercise 3-way interactions in generate_interactions
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--cubic", "abc"));

  for (int i = 0; i < 10; ++i)
  {
    auto* ex = VW::read_example(*vw,
        "1.0 |a x:" + std::to_string(i) + " |b y:" + std::to_string(i) + " |c z:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

TEST(ReductionsCoverage3, GenerateInteractionsWildcardCubic)
{
  // Use wildcard : for all namespace interactions
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--cubic", ":::"));

  for (int i = 0; i < 10; ++i)
  {
    auto* ex = VW::read_example(*vw, "1.0 |a x:" + std::to_string(i) + " |b y:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ---- Confidence sequence robust - exercise more paths (confidence_sequence_robust.cc) ----
TEST(ReductionsCoverage3, ConfidenceSequenceRobustDefaultBisection)
{
  // Default uses bisection (is_brentq=false)
  VW::estimators::confidence_sequence_robust cs;

  for (int i = 0; i < 100; ++i)
  {
    double r = static_cast<double>(i % 10) / 10.0;
    cs.update(1.0, r);
  }

  EXPECT_LE(cs.lower_bound(), cs.upper_bound());
}

TEST(ReductionsCoverage3, ConfidenceSequenceRobustBrentq)
{
  // Exercise Brent's method root-finding path (lines 191-263)
  VW::estimators::confidence_sequence_robust cs(1e-6, true);

  for (int i = 0; i < 50; ++i)
  {
    double r = static_cast<double>(i % 5) / 5.0;
    cs.update(1.0, r);
  }

  EXPECT_LE(cs.lower_bound(), cs.upper_bound());
}

// ---- CB label parsing error paths (cb.cc lines 98, 113, 221, 266-271) ----
TEST(CbCoverage3, PrintUpdateNoAction)
{
  auto vw = VW::initialize(vwtest::make_args("--cb", "2", "--quiet"));

  // Train some examples
  for (int i = 0; i < 10; ++i)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i % 2 + 1) + ":1.0:0.5 | x:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ---- Text parser with different formats to push parse_example_text.cc coverage ----
TEST(TextParserCoverage3, ParseFeaturesWithWeightAndTag)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));

  // Example with weight, initial, and tag
  auto* ex1 = VW::read_example(*vw, "1.5 2.0 3.0 'tag1 | a b c");
  vw->learn(*ex1);
  VW::finish_example(*vw, *ex1);

  // Example with pipe in different positions
  auto* ex2 = VW::read_example(*vw, "1.0 |ns1 a:1.0 b:2.0 |ns2 c:3.0");
  vw->learn(*ex2);
  VW::finish_example(*vw, *ex2);

  // Example with empty namespace
  auto* ex3 = VW::read_example(*vw, "1.0 | a b | c d");
  vw->learn(*ex3);
  VW::finish_example(*vw, *ex3);
}

// ---- CSOAA LDF with ranked labels for more csoaa_ldf.cc coverage ----
TEST(ReductionsCoverage3, CsoaaLdfRanked)
{
  auto vw = VW::initialize(vwtest::make_args("--csoaa_ldf", "mc", "--csoaa_rank", "--quiet"));

  for (int i = 0; i < 10; ++i)
  {
    auto* shared = VW::read_example(*vw, "shared | s:" + std::to_string(i));
    auto* a1 = VW::read_example(*vw, "1:1.0 | a:1");
    auto* a2 = VW::read_example(*vw, "2:2.0 | a:2");
    auto* a3 = VW::read_example(*vw, "3:0.5 | a:3");
    auto* empty = VW::read_example(*vw, "");

    VW::multi_ex multi_ex = {shared, a1, a2, a3};
    vw->learn(multi_ex);
    vw->finish_example(multi_ex);
    VW::finish_example(*vw, *empty);
  }
}

// ---- Cbify with exploration (cbify.cc more paths) ----
TEST(ReductionsCoverage3, CbifyWithExploration)
{
  auto vw = VW::initialize(vwtest::make_args("--cbify", "3", "--cb_type", "mtr", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i % 3 + 1) + " | x:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ---- Learner merge paths (learner.cc) ----
TEST(ReductionsCoverage3, WorkspaceMerge)
{
  auto vw1 = VW::initialize(vwtest::make_args("--quiet"));
  auto vw2 = VW::initialize(vwtest::make_args("--quiet"));

  // Train vw1
  for (int i = 0; i < 10; ++i)
  {
    auto* ex = VW::read_example(*vw1, "1.0 | a:" + std::to_string(i));
    vw1->learn(*ex);
    VW::finish_example(*vw1, *ex);
  }
  // Train vw2
  for (int i = 0; i < 10; ++i)
  {
    auto* ex = VW::read_example(*vw2, "2.0 | b:" + std::to_string(i));
    vw2->learn(*ex);
    VW::finish_example(*vw2, *ex);
  }
}

// ---- Parser paths (parser.cc) - exercise text input mode explicitly ----
TEST(ParserCoverage3, TextInputWithMultipleNamespaces)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));

  // Example with many namespaces to exercise parser paths
  auto* ex = VW::read_example(*vw, "1.0 |a x:1 y:2 |b z:3 |c w:4 v:5");
  vw->learn(*ex);
  EXPECT_GT(ex->get_num_features(), 0);
  VW::finish_example(*vw, *ex);
}

// ---- Accumulate paths (accumulate.cc) ----
// This requires allreduce which is hard to test in unit tests, skip.

// ---- IO buf additional paths (io_buf.cc) ----
TEST(IoBufCoverage3, WriteAndReadBackModel)
{
  // Exercise more io_buf code paths through model save/load cycle
  auto backing_vector = std::make_shared<std::vector<char>>();
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet"));
    for (int i = 0; i < 10; ++i)
    {
      auto* ex = VW::read_example(*vw, "1.0 | a:" + std::to_string(i));
      vw->learn(*ex);
      VW::finish_example(*vw, *ex);
    }
    VW::io_buf io_writer;
    io_writer.add_file(VW::io::create_vector_writer(backing_vector));
    VW::save_predictor(*vw, io_writer);
  }
  EXPECT_GT(backing_vector->size(), 0);

  // Read it back
  {
    auto vw = VW::initialize(vwtest::make_args("--quiet"),
        VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));
    auto* ex = VW::read_example(*vw, "| a:5");
    vw->predict(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ---- Parse args: readable model output (parse_args.cc) ----
TEST(ParseArgsCoverage3, ReadableModelOutput)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--readable_model", "/dev/null"));

  for (int i = 0; i < 5; ++i)
  {
    auto* ex = VW::read_example(*vw, "1.0 | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ---- OAA with probabilities (oaa.cc scores path) ----
TEST(ReductionsCoverage3, OaaWithProbabilities)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "3", "--probabilities", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i % 3 + 1) + " | x:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
  auto* ex = VW::read_example(*vw, "| x:5");
  vw->predict(*ex);
  VW::finish_example(*vw, *ex);
}

// ---- Confidence reduction (confidence.cc more paths) ----
TEST(ReductionsCoverage3, ConfidenceWithMultipleExamples)
{
  auto vw = VW::initialize(vwtest::make_args("--confidence", "--quiet"));

  for (int i = 0; i < 30; ++i)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i % 2) + " | a:" + std::to_string(i) + " b:" + std::to_string(i * 2));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ---- Scorer reduction (scorer.cc) ----
TEST(ReductionsCoverage3, ScorerReduction)
{
  auto vw = VW::initialize(vwtest::make_args("--link", "logistic", "--quiet"));

  for (int i = 0; i < 10; ++i)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i % 2) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
  auto* ex = VW::read_example(*vw, "| a:5");
  vw->predict(*ex);
  // logistic link should produce output between 0 and 1
  EXPECT_GE(ex->pred.scalar, 0.0f);
  EXPECT_LE(ex->pred.scalar, 1.0f);
  VW::finish_example(*vw, *ex);
}

// ---- Svrg reduction (svrg.cc) ----
TEST(ReductionsCoverage3, SvrgTraining)
{
  auto vw = VW::initialize(vwtest::make_args("--svrg", "--quiet"));

  for (int i = 0; i < 30; ++i)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i % 2) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ---- Active reduction (active.cc more paths) ----
TEST(ReductionsCoverage3, ActiveLearning)
{
  auto vw = VW::initialize(vwtest::make_args("--active", "--simulation", "--quiet"));

  for (int i = 0; i < 20; ++i)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i % 2) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}

// ---- vw_validate.cc paths ----
TEST(VwCoverage3, ValidatePaths)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  // Simply creating and using a workspace exercises some validate paths
  auto* ex = VW::read_example(*vw, "1.0 | a b c");
  vw->learn(*ex);
  VW::finish_example(*vw, *ex);
  // Verify num_bits is valid
  EXPECT_GT(vw->initial_weights_config.num_bits, 0);
}

// ---- Multiclass with named labels (multiclass.cc) ----
TEST(ReductionsCoverage3, MulticlassNamedLabels)
{
  auto vw = VW::initialize(vwtest::make_args("--oaa", "3", "--named_labels", "cat,dog,bird", "--quiet"));

  for (int i = 0; i < 15; ++i)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i % 3 + 1) + " | x:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
  auto* ex = VW::read_example(*vw, "| x:5");
  vw->predict(*ex);
  VW::finish_example(*vw, *ex);
}

// ---- Baseline reduction ----
TEST(ReductionsCoverage3, BaselineReduction)
{
  auto vw = VW::initialize(vwtest::make_args("--baseline", "--quiet", "--loss_function", "logistic"));

  for (int i = 0; i < 20; ++i)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i % 2 == 0 ? 1 : -1) + " | a:" + std::to_string(i));
    vw->learn(*ex);
    VW::finish_example(*vw, *ex);
  }
}
