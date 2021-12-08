// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "logger.h"

namespace VW
{
namespace io
{

void logger::set_level(log_level lvl)
{
  _logger_impl->_spdlog_logger->set_level(static_cast<spdlog::level::level_enum>(lvl));
}

void logger::set_max_output(size_t max) { _logger_impl->_max_limit = max; }

size_t logger::get_log_count() const { return _logger_impl->_log_count; }

void logger::log_summary()
{
  if (_logger_impl->_max_limit != SIZE_MAX && _logger_impl->_log_count > _logger_impl->_max_limit)
  {
    critical(
        "Omitted some log lines. Re-run without --limit_output N for full log. Total log lines: {}", _logger_impl->_log_count);
  }
}

}  // namespace io
}  // namespace VW
