// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/estimators/cressieread.h"

#include "vw/core/io_buf.h"
#include "vw/core/metric_sink.h"
#include "vw/core/model_utils.h"
#include "vw/io/io_adapter.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(CressiereadTest, DefaultConstruction)
{
  VW::estimators::cressieread cr;

  EXPECT_EQ(cr.update_count, 0u);
  EXPECT_FLOAT_EQ(cr.ips, 0.0f);
  EXPECT_FLOAT_EQ(cr.last_w, 0.0f);
  EXPECT_FLOAT_EQ(cr.last_r, 0.0f);
}

TEST(CressiereadTest, CustomConstruction)
{
  VW::estimators::cressieread cr(0.1, 0.5);

  EXPECT_EQ(cr.update_count, 0u);
  EXPECT_FLOAT_EQ(cr.ips, 0.0f);
}

TEST(CressiereadTest, Update)
{
  VW::estimators::cressieread cr;

  cr.update(0.5f, 1.0f);

  EXPECT_EQ(cr.update_count, 1u);
  EXPECT_FLOAT_EQ(cr.ips, 0.5f);  // r * w = 1.0 * 0.5
  EXPECT_FLOAT_EQ(cr.last_w, 0.5f);
  EXPECT_FLOAT_EQ(cr.last_r, 1.0f);

  cr.update(0.25f, 2.0f);

  EXPECT_EQ(cr.update_count, 2u);
  EXPECT_FLOAT_EQ(cr.ips, 1.0f);  // 0.5 + 2.0 * 0.25 = 1.0
  EXPECT_FLOAT_EQ(cr.last_w, 0.25f);
  EXPECT_FLOAT_EQ(cr.last_r, 2.0f);
}

TEST(CressiereadTest, CurrentIpsEmpty)
{
  VW::estimators::cressieread cr;

  // When update_count is 0, current_ips should return 0
  EXPECT_FLOAT_EQ(cr.current_ips(), 0.0f);
}

TEST(CressiereadTest, CurrentIps)
{
  VW::estimators::cressieread cr;

  cr.update(0.5f, 1.0f);
  EXPECT_FLOAT_EQ(cr.current_ips(), 0.5f);  // 0.5 / 1

  cr.update(0.5f, 1.0f);
  EXPECT_FLOAT_EQ(cr.current_ips(), 0.5f);  // 1.0 / 2

  cr.update(1.0f, 2.0f);
  EXPECT_FLOAT_EQ(cr.current_ips(), 1.0f);  // 3.0 / 3
}

TEST(CressiereadTest, ResetStats)
{
  VW::estimators::cressieread cr;

  cr.update(0.5f, 1.0f);
  cr.update(0.25f, 2.0f);

  EXPECT_GT(cr.update_count, 0u);
  EXPECT_NE(cr.ips, 0.0f);

  cr.reset_stats();

  EXPECT_EQ(cr.update_count, 0u);
  EXPECT_FLOAT_EQ(cr.ips, 0.0f);
  EXPECT_FLOAT_EQ(cr.last_w, 0.0f);
  EXPECT_FLOAT_EQ(cr.last_r, 0.0f);
}

TEST(CressiereadTest, ResetStatsWithCustomParams)
{
  VW::estimators::cressieread cr;

  cr.update(0.5f, 1.0f);
  cr.reset_stats(0.1, 0.5);

  EXPECT_EQ(cr.update_count, 0u);
  EXPECT_FLOAT_EQ(cr.ips, 0.0f);
}

TEST(CressiereadTest, Bounds)
{
  VW::estimators::cressieread cr;

  // Add some data to get meaningful bounds
  cr.update(0.5f, 1.0f);
  cr.update(0.75f, 0.5f);
  cr.update(1.0f, 0.8f);

  // Bounds should exist and lower <= upper
  float lower = cr.lower_bound();
  float upper = cr.upper_bound();

  EXPECT_LE(lower, upper);
}

TEST(CressiereadTest, Persist)
{
  VW::estimators::cressieread cr;

  cr.update(0.5f, 1.0f);
  cr.update(0.75f, 0.5f);

  VW::metric_sink metrics;
  cr.persist(metrics, "_test");

  // Verify metrics were set
  EXPECT_EQ(metrics.get_uint("upcnt_test"), 2u);
  EXPECT_FLOAT_EQ(metrics.get_float("ips_test"), cr.current_ips());
  EXPECT_FLOAT_EQ(metrics.get_float("w_test"), 0.75f);
  EXPECT_FLOAT_EQ(metrics.get_float("r_test"), 0.5f);
}

TEST(CressiereadTest, ModelSerialization)
{
  VW::estimators::cressieread cr;
  cr.update(0.5f, 1.0f);
  cr.update(0.75f, 0.5f);
  cr.update(1.0f, 0.8f);

  // Write to buffer
  auto buffer = std::make_shared<std::vector<char>>();
  VW::io_buf write_buf;
  write_buf.add_file(VW::io::create_vector_writer(buffer));

  size_t bytes_written = VW::model_utils::write_model_field(write_buf, cr, "test", false);
  write_buf.flush();

  EXPECT_GT(bytes_written, 0u);

  // Read back
  VW::estimators::cressieread cr2;
  VW::io_buf read_buf;
  read_buf.add_file(VW::io::create_buffer_view(buffer->data(), buffer->size()));

  size_t bytes_read = VW::model_utils::read_model_field(read_buf, cr2);

  EXPECT_EQ(bytes_read, bytes_written);
  EXPECT_EQ(cr2.update_count, cr.update_count);
  EXPECT_FLOAT_EQ(cr2.ips, cr.ips);
  EXPECT_FLOAT_EQ(cr2.last_w, cr.last_w);
  EXPECT_FLOAT_EQ(cr2.last_r, cr.last_r);
}
