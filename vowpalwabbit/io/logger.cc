// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "logger.h"
#include "spdlog/sinks/stdout_sinks.h"

namespace VW
{
namespace io
{
namespace logger
{
// FIXME: the get() call returns a shared_ptr. Keep a copy here to avoid unnecessary shared_ptr copies
// This can go away once we move to an object-based logger
namespace detail{
  std::shared_ptr<spdlog::logger> _stderr_logger = spdlog::stderr_logger_st("stderr");
  const constexpr char* default_pattern = "[%l] %v";
  size_t max_limit;
  size_t log_count;
}

void log_set_level(log_level lvl)
{
  spdlog::set_level(static_cast<spdlog::level::level_enum>(lvl));
}

void set_max_output(size_t max) { detail::max_limit = max; }

pattern_guard::pattern_guard(const std::string& pattern)
{
  spdlog::set_pattern(pattern);
}

pattern_guard::~pattern_guard()
{
  spdlog::set_pattern(detail::default_pattern);
}

void initialize_logger()
{
  detail::max_limit = SIZE_MAX;
  spdlog::set_pattern(detail::default_pattern);
}

size_t get_log_count() { return detail::log_count; }

void log_summary()
{
  if (detail::max_limit != SIZE_MAX && detail::log_count > detail::max_limit)
  {
    logger::errlog_critical(
        "Ommitted some log lines. Re-run without --limit_output N for full log. Total {}", detail::log_count);
  }
}
}
}
}
