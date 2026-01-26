// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/estimators/confidence_sequence.h"

#include "vw/core/io_buf.h"
#include "vw/core/metric_sink.h"
#include "vw/core/model_utils.h"
#include "vw/io/io_adapter.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cmath>
#include <memory>
#include <vector>

constexpr float CS_FLOAT_TOL = 0.1f;

TEST(ConfidenceSeq, IncrementalFsumTest)
{
  VW::details::incremental_f_sum fsum;
  fsum += 1.0;
  fsum += 2.0;
  fsum += 3.0;
  fsum += 4.0;
  fsum += 5.0;
  fsum += 6.0;
  fsum += 7.0;
  fsum += 8.0;
  fsum += 9.0;
  fsum += 10.0;
  EXPECT_FLOAT_EQ(fsum, 55.0);
}

TEST(ConfidenceSeq, IncrementalFsumAddition)
{
  VW::details::incremental_f_sum fsum1;
  fsum1 += 1.0;
  fsum1 += 2.0;
  fsum1 += 3.0;

  VW::details::incremental_f_sum fsum2;
  fsum2 += 4.0;
  fsum2 += 5.0;

  VW::details::incremental_f_sum result = fsum1 + fsum2;
  EXPECT_FLOAT_EQ(static_cast<double>(result), 15.0);
}

TEST(ConfidenceSeq, IncrementalFsumMixedMagnitudes)
{
  VW::details::incremental_f_sum fsum;
  // Add values of very different magnitudes to test Kahan summation
  fsum += 1e10;
  fsum += 1.0;
  fsum += 1e-10;
  fsum += 1e10;

  // The result should be accurate despite magnitude differences
  double result = static_cast<double>(fsum);
  EXPECT_NEAR(result, 2e10 + 1.0 + 1e-10, 1e-5);
}

TEST(ConfidenceSeq, ConfidenceSequenceTest)
{
  VW::estimators::confidence_sequence cs;
  std::vector<double> rs;
  for (int i = 0; i < 1000; ++i)
  {
    rs.push_back(0.5);
    rs.push_back(0.6);
    rs.push_back(0.7);
    rs.push_back(0.8);
  }

  for (double r : rs) { cs.update(r, r); }

  float lb = cs.lower_bound();
  float ub = cs.upper_bound();

  // Compare to test_confidence_sequence.py
  EXPECT_NEAR(lb, 0.4215480f, CS_FLOAT_TOL);
  EXPECT_NEAR(ub, 0.7907692f, CS_FLOAT_TOL);
}

TEST(ConfidenceSeq, BoundsAtZeroT)
{
  VW::estimators::confidence_sequence cs;
  // At t=0, bounds should return rmin and rmax
  float lb = cs.lower_bound();
  float ub = cs.upper_bound();
  // Default rmin_init and rmax_init
  EXPECT_LE(lb, ub);
}

TEST(ConfidenceSeq, ResetStats)
{
  VW::estimators::confidence_sequence cs;

  // Update a few times
  cs.update(1.0, 0.5);
  cs.update(1.0, 0.6);
  cs.update(1.0, 0.7);

  // Reset
  cs.reset_stats();

  // After reset, bounds should be back to initial values
  float lb = cs.lower_bound();
  float ub = cs.upper_bound();
  EXPECT_LE(lb, ub);
}

TEST(ConfidenceSeq, UpdateWithPDrop)
{
  VW::estimators::confidence_sequence cs;

  // Update with p_drop parameter
  cs.update(1.0, 0.5, 0.1);  // 10% dropout
  cs.update(1.0, 0.6, 0.2);  // 20% dropout

  float lb = cs.lower_bound();
  float ub = cs.upper_bound();
  EXPECT_LE(lb, ub);
  EXPECT_FALSE(std::isnan(lb));
  EXPECT_FALSE(std::isnan(ub));
}

TEST(ConfidenceSeq, UpdateWithNDrop)
{
  VW::estimators::confidence_sequence cs;

  // Update with explicit n_drop parameter
  cs.update(1.0, 0.5, 0.0, 2.0);  // n_drop = 2
  cs.update(1.0, 0.6, 0.0, 1.0);  // n_drop = 1

  float lb = cs.lower_bound();
  float ub = cs.upper_bound();
  EXPECT_LE(lb, ub);
  EXPECT_FALSE(std::isnan(lb));
  EXPECT_FALSE(std::isnan(ub));
}

TEST(ConfidenceSeq, AdjustFalseMode)
{
  // Create with adjust=false (clamping mode)
  VW::estimators::confidence_sequence cs(0.05, 0.0, 1.0, false);

  // Values outside [rmin, rmax] should be clamped
  cs.update(1.0, -0.5);  // Below rmin
  cs.update(1.0, 1.5);   // Above rmax
  cs.update(1.0, 0.5);   // Within range

  float lb = cs.lower_bound();
  float ub = cs.upper_bound();
  EXPECT_GE(lb, 0.0f);
  EXPECT_LE(ub, 1.0f);
}

TEST(ConfidenceSeq, AdjustTrueMode)
{
  // Create with adjust=true (default - tracking mode)
  VW::estimators::confidence_sequence cs(0.05, 0.0, 1.0, true);

  // Values outside initial range should expand the range
  cs.update(1.0, -0.5);  // Below initial rmin
  cs.update(1.0, 1.5);   // Above initial rmax

  float lb = cs.lower_bound();
  float ub = cs.upper_bound();
  // With adjust=true, bounds can go outside initial [0, 1]
  EXPECT_FALSE(std::isnan(lb));
  EXPECT_FALSE(std::isnan(ub));
}

TEST(ConfidenceSeq, PersistMetrics)
{
  VW::estimators::confidence_sequence cs;
  cs.update(1.0, 0.5);
  cs.update(1.0, 0.6);

  VW::metric_sink metrics;
  cs.persist(metrics, "_test");

  // Verify metrics were set
  EXPECT_NO_THROW(metrics.get_uint("upcnt_test"));
  EXPECT_NO_THROW(metrics.get_float("lb_test"));
  EXPECT_NO_THROW(metrics.get_float("ub_test"));
  EXPECT_NO_THROW(metrics.get_float("last_w_test"));
  EXPECT_NO_THROW(metrics.get_float("last_r_test"));

  EXPECT_EQ(metrics.get_uint("upcnt_test"), 2);
}

TEST(ConfidenceSeq, ModelSerialization)
{
  VW::estimators::confidence_sequence original(0.05, 0.0, 1.0, true);
  original.update(1.0, 0.5);
  original.update(1.0, 0.6);
  original.update(1.0, 0.7);

  float orig_lb = original.lower_bound();
  float orig_ub = original.upper_bound();

  // Serialize
  VW::io_buf write_buffer;
  auto backing_buffer_write = std::make_shared<std::vector<char>>();
  write_buffer.add_file(VW::io::create_vector_writer(backing_buffer_write));
  size_t written = VW::model_utils::write_model_field(write_buffer, original, "cs_test", false);
  write_buffer.flush();
  EXPECT_GT(written, 0);

  // Deserialize
  VW::io_buf read_buffer;
  read_buffer.add_file(VW::io::create_buffer_view(backing_buffer_write->data(), backing_buffer_write->size()));
  VW::estimators::confidence_sequence restored;
  size_t read = VW::model_utils::read_model_field(read_buffer, restored);
  EXPECT_GT(read, 0);

  // Verify restored values match
  EXPECT_FLOAT_EQ(restored.lower_bound(), orig_lb);
  EXPECT_FLOAT_EQ(restored.upper_bound(), orig_ub);
  EXPECT_EQ(restored.update_count, original.update_count);
}

TEST(ConfidenceSeq, IncrementalFSumSerialization)
{
  VW::details::incremental_f_sum original;
  original += 1.0;
  original += 2.0;
  original += 3.0;

  // Serialize
  VW::io_buf write_buffer;
  auto backing_buffer_write = std::make_shared<std::vector<char>>();
  write_buffer.add_file(VW::io::create_vector_writer(backing_buffer_write));
  size_t written = VW::model_utils::write_model_field(write_buffer, original, "fsum_test", false);
  write_buffer.flush();
  EXPECT_GT(written, 0);

  // Deserialize
  VW::io_buf read_buffer;
  read_buffer.add_file(VW::io::create_buffer_view(backing_buffer_write->data(), backing_buffer_write->size()));
  VW::details::incremental_f_sum restored;
  size_t read = VW::model_utils::read_model_field(read_buffer, restored);
  EXPECT_GT(read, 0);

  EXPECT_FLOAT_EQ(static_cast<double>(restored), static_cast<double>(original));
}

TEST(ConfidenceSeq, CustomAlpha)
{
  // Test with different alpha values
  VW::estimators::confidence_sequence cs_low_alpha(0.01, 0.0, 1.0, true);
  VW::estimators::confidence_sequence cs_high_alpha(0.10, 0.0, 1.0, true);

  for (int i = 0; i < 100; ++i)
  {
    cs_low_alpha.update(1.0, 0.5);
    cs_high_alpha.update(1.0, 0.5);
  }

  // Lower alpha should give wider confidence intervals
  float width_low = cs_low_alpha.upper_bound() - cs_low_alpha.lower_bound();
  float width_high = cs_high_alpha.upper_bound() - cs_high_alpha.lower_bound();
  EXPECT_GE(width_low, width_high);
}

TEST(ConfidenceSeq, RminEqualsRmax)
{
  // Edge case: rmin == rmax
  VW::estimators::confidence_sequence cs(0.05, 0.5, 0.5, true);

  cs.update(1.0, 0.5);

  // When rmin == rmax, bounds should return rmin/rmax
  float lb = cs.lower_bound();
  float ub = cs.upper_bound();
  EXPECT_FLOAT_EQ(lb, 0.5f);
  EXPECT_FLOAT_EQ(ub, 0.5f);
}