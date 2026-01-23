// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/vw.h"
#include "vw/io/logger.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace
{
// Helper to capture log output
class log_capture
{
public:
  std::vector<std::string> warnings;
  std::vector<std::string> errors;

  static void log_callback(void* context, VW::io::log_level level, const std::string& message)
  {
    auto* capture = static_cast<log_capture*>(context);
    if (level == VW::io::log_level::WARN_LEVEL) { capture->warnings.push_back(message); }
    else if (level == VW::io::log_level::ERROR_LEVEL) { capture->errors.push_back(message); }
  }

  bool has_warning_containing(const std::string& substring) const
  {
    for (const auto& warning : warnings)
    {
      if (warning.find(substring) != std::string::npos) { return true; }
    }
    return false;
  }
};
}  // namespace

TEST(CbAdf, WarnsWhenDsjsonWithoutInteractions)
{
  log_capture capture;
  auto logger = VW::io::create_custom_sink_logger(&capture, log_capture::log_callback);

  // cb_adf with dsjson but no interactions should warn
  auto vw = VW::initialize(vwtest::make_args("--cb_adf", "--dsjson", "--no_stdin"), nullptr, nullptr, nullptr, &logger);

  EXPECT_TRUE(capture.has_warning_containing("cb_adf is used with JSON input but without any interaction features"));
  vw->finish();
}

TEST(CbAdf, NoWarningWhenDsjsonWithQuadratic)
{
  log_capture capture;
  auto logger = VW::io::create_custom_sink_logger(&capture, log_capture::log_callback);

  // cb_adf with dsjson AND quadratic should NOT warn
  auto vw =
      VW::initialize(vwtest::make_args("--cb_adf", "--dsjson", "--no_stdin", "-q", "::"), nullptr, nullptr, nullptr, &logger);

  EXPECT_FALSE(capture.has_warning_containing("cb_adf is used with JSON input but without any interaction features"));
  vw->finish();
}

TEST(CbAdf, NoWarningWhenDsjsonWithInteractions)
{
  log_capture capture;
  auto logger = VW::io::create_custom_sink_logger(&capture, log_capture::log_callback);

  // cb_adf with dsjson AND --interactions should NOT warn
  auto vw = VW::initialize(
      vwtest::make_args("--cb_adf", "--dsjson", "--no_stdin", "--interactions", "AB"), nullptr, nullptr, nullptr, &logger);

  EXPECT_FALSE(capture.has_warning_containing("cb_adf is used with JSON input but without any interaction features"));
  vw->finish();
}

TEST(CbAdf, NoWarningWithoutJsonInput)
{
  log_capture capture;
  auto logger = VW::io::create_custom_sink_logger(&capture, log_capture::log_callback);

  // cb_adf without JSON input should NOT warn (even without interactions)
  auto vw = VW::initialize(vwtest::make_args("--cb_adf", "--no_stdin"), nullptr, nullptr, nullptr, &logger);

  EXPECT_FALSE(capture.has_warning_containing("cb_adf is used with JSON input but without any interaction features"));
  vw->finish();
}

TEST(CbAdf, NoWarningForHigherLevelReductions)
{
  log_capture capture;
  auto logger = VW::io::create_custom_sink_logger(&capture, log_capture::log_callback);

  // cb_explore_adf with dsjson but no interactions should NOT warn
  // because it's a higher-level reduction that uses cb_adf internally
  auto vw = VW::initialize(
      vwtest::make_args("--cb_explore_adf", "--dsjson", "--no_stdin"), nullptr, nullptr, nullptr, &logger);

  EXPECT_FALSE(capture.has_warning_containing("cb_adf is used with JSON input but without any interaction features"));
  vw->finish();
}
