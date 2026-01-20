// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/metric_sink.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(Ftrl, FtrlMetricsAreExposed)
{
  auto vw = VW::initialize(vwtest::make_args("--ftrl", "--extra_metrics", "--quiet"));

  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);

  ex = VW::read_example(*vw, "-1 | d e f");
  vw->learn(*ex);
  vw->finish_example(*ex);

  auto metrics = vw->output_runtime.global_metrics.collect_metrics(vw->l.get());

  EXPECT_EQ(metrics.get_string("ftrl_algorithm"), "Proximal-FTRL");
  EXPECT_FLOAT_EQ(metrics.get_float("ftrl_alpha"), 0.5f);  // default for FTRL
  EXPECT_FLOAT_EQ(metrics.get_float("ftrl_beta"), 1.0f);   // default for FTRL
  EXPECT_EQ(metrics.get_uint("ftrl_size"), 3);
}

TEST(Ftrl, PistolMetricsAreExposed)
{
  auto vw = VW::initialize(vwtest::make_args("--pistol", "--extra_metrics", "--quiet"));

  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);

  ex = VW::read_example(*vw, "-1 | d e f");
  vw->learn(*ex);
  vw->finish_example(*ex);

  auto metrics = vw->output_runtime.global_metrics.collect_metrics(vw->l.get());

  EXPECT_EQ(metrics.get_string("ftrl_algorithm"), "PiSTOL");
  EXPECT_FLOAT_EQ(metrics.get_float("ftrl_alpha"), 1.0f);  // default for PiSTOL
  EXPECT_FLOAT_EQ(metrics.get_float("ftrl_beta"), 0.5f);   // default for PiSTOL
  EXPECT_EQ(metrics.get_uint("ftrl_size"), 4);
}

TEST(Ftrl, CoinMetricsAreExposed)
{
  auto vw = VW::initialize(vwtest::make_args("--coin", "--extra_metrics", "--quiet"));

  auto* ex = VW::read_example(*vw, "1 | a b c");
  vw->learn(*ex);
  vw->finish_example(*ex);

  ex = VW::read_example(*vw, "-1 | d e f");
  vw->learn(*ex);
  vw->finish_example(*ex);

  auto metrics = vw->output_runtime.global_metrics.collect_metrics(vw->l.get());

  EXPECT_EQ(metrics.get_string("ftrl_algorithm"), "Coin Betting");
  EXPECT_FLOAT_EQ(metrics.get_float("ftrl_alpha"), 4.0f);  // default for Coin
  EXPECT_FLOAT_EQ(metrics.get_float("ftrl_beta"), 1.0f);   // default for Coin
  EXPECT_EQ(metrics.get_uint("ftrl_size"), 6);
}
