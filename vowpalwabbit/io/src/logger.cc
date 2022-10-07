// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/io/logger.h"

namespace VW
{
namespace io
{
void logger::set_level(log_level lvl)
{
  _logger_impl->spdlog_stdout_logger->set_level(static_cast<spdlog::level::level_enum>(lvl));
  _logger_impl->spdlog_stderr_logger->set_level(static_cast<spdlog::level::level_enum>(lvl));
}

void logger::set_max_output(size_t max) { _logger_impl->max_limit = max; }

size_t logger::get_log_count() const { return _logger_impl->log_count; }

void logger::set_location(output_location location) { _logger_impl->location = location; }

void logger::log_summary()
{
  if (_logger_impl->max_limit != SIZE_MAX && _logger_impl->log_count > _logger_impl->max_limit)
  {
    err_critical("Omitted some log lines. Re-run without --limit_output N for full log. Total log lines: {}",
        _logger_impl->log_count);
  }
}

output_location get_output_location(const std::string& name)
{
  if (name == "stdout") { return output_location::STDOUT; }
  if (name == "stderr") { return output_location::STDERR; }
  if (name == "compat") { return output_location::COMPAT; }

  THROW("invalid output location: " << name);
}

log_level get_log_level(const std::string& level)
{
  if (level == "trace") { return log_level::TRACE_LEVEL; }
  if (level == "debug") { return log_level::DEBUG_LEVEL; }
  if (level == "info") { return log_level::INFO_LEVEL; }
  if (level == "warn") { return log_level::WARN_LEVEL; }
  if (level == "error") { return log_level::ERROR_LEVEL; }
  if (level == "critical") { return log_level::CRITICAL_LEVEL; }
  if (level == "off") { return log_level::OFF_LEVEL; }
  THROW("invalid log level: " << level);
}

}  // namespace io
}  // namespace VW
