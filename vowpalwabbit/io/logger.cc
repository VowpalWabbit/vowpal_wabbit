// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "logger.h"

namespace VW
{
namespace io
{
namespace details
{
  template <typename Mutex>
class function_ptr_sink : public spdlog::sinks::base_sink<Mutex>
{
public:
  using func_t = void (*)(void*, const std::string&);
  function_ptr_sink(void* context, func_t func) : spdlog::sinks::base_sink<Mutex>(), _func(func), _context(context) {}

protected:
  void sink_it_(const spdlog::details::log_msg& msg) override
  {
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
    _func(_context, fmt::to_string(formatted));
  }

  void flush_() override {}

  func_t _func;
  void* _context;
};
}

void logger::set_level(log_level lvl)
{
  _logger_impl->_spdlog_stdout_logger->set_level(static_cast<spdlog::level::level_enum>(lvl));
  _logger_impl->_spdlog_stderr_logger->set_level(static_cast<spdlog::level::level_enum>(lvl));
}

void logger::set_max_output(size_t max) { _logger_impl->_max_limit = max; }

size_t logger::get_log_count() const { return _logger_impl->_log_count; }

void logger::set_location(output_location location) { _logger_impl->_location = location; }

void logger::log_summary()
{
  if (_logger_impl->_max_limit != SIZE_MAX && _logger_impl->_log_count > _logger_impl->_max_limit)
  {
    _logger_impl->critical(
        "Omitted some log lines. Re-run without --limit_output N for full log. Total log lines: {}",
        _logger_impl->_log_count);
  }
}

output_location get_output_location(const std::string& name)
{
  if (name == "stdout") { return output_location::out; }
  if (name == "stderr") { return output_location::err; }
  THROW("invalid output location: " << name);
}

log_level get_log_level(const std::string& level)
{
  if (level == "trace") { return log_level::trace; }
  if (level == "debug") { return log_level::debug; }
  if (level == "info") { return log_level::info; }
  if (level == "warn") { return log_level::warn; }
  if (level == "error") { return log_level::error; }
  if (level == "critical") { return log_level::critical; }
  if (level == "off") { return log_level::off; }
  THROW("invalid log level: " << level);
}

logger create_default_logger()
{
  auto stdout_sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
  auto stderr_sink = std::make_shared<spdlog::sinks::stderr_sink_mt>();
  return logger(std::make_shared<details::logger_impl>(
      std::unique_ptr<spdlog::logger>(new spdlog::logger("vowpal-stdout", stdout_sink)),
      std::unique_ptr<spdlog::logger>(new spdlog::logger("vowpal-stderr", stderr_sink))

          ));
}

logger create_null_logger()
{
  auto null_out_sink = std::make_shared<spdlog::sinks::null_sink_st>();
  auto null_err_sink = std::make_shared<spdlog::sinks::null_sink_st>();
  return logger(std::make_shared<details::logger_impl>(
      std::unique_ptr<spdlog::logger>(new spdlog::logger("vowpal-stdout", null_out_sink)),
      std::unique_ptr<spdlog::logger>(new spdlog::logger("vowpal-stderr", null_err_sink))

          ));
}

logger create_custom_sink_logger(void* context, void (*func)(void*, const std::string&))
{
  auto fptr_out_sink = std::make_shared<details::function_ptr_sink<std::mutex>>(context, func);
  auto fptr_err_sink = std::make_shared<details::function_ptr_sink<std::mutex>>(context, func);
  return logger(std::make_shared<details::logger_impl>(
      std::unique_ptr<spdlog::logger>(new spdlog::logger("vowpal-stdout", fptr_out_sink)),
      std::unique_ptr<spdlog::logger>(new spdlog::logger("vowpal-stdout", fptr_err_sink))));
}

}  // namespace io
}  // namespace VW
