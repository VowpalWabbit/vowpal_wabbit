// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/io/logger.h"

#include "vw/common/vw_exception.h"
#include "vw/common/vw_throw.h"

#include <spdlog/logger.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>

#include <memory>

namespace
{
const constexpr char* DEFAULT_PATTERN = "%^[%l]%$ %v";

template <typename Mutex>
class function_ptr_sink : public spdlog::sinks::base_sink<Mutex>
{
public:
  function_ptr_sink(void* context, VW::io::logger_output_func_t func)
      : spdlog::sinks::base_sink<Mutex>(), _func(func), _context(context)
  {
  }

protected:
  void sink_it_(const spdlog::details::log_msg& msg) override
  {
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
    _func(_context, static_cast<VW::io::log_level>(msg.level), fmt::to_string(formatted));
  }

  void flush_() override {}

  VW::io::logger_output_func_t _func;
  void* _context;
};

// Same as above but ignores the log level.
template <typename Mutex>
class function_ptr_legacy_sink : public spdlog::sinks::base_sink<Mutex>
{
public:
  function_ptr_legacy_sink(void* context, VW::io::logger_legacy_output_func_t func)
      : spdlog::sinks::base_sink<Mutex>(), _func(func), _context(context)
  {
  }

protected:
  void sink_it_(const spdlog::details::log_msg& msg) override
  {
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
    _func(_context, fmt::to_string(formatted));
  }

  void flush_() override {}

  VW::io::logger_legacy_output_func_t _func;
  void* _context;
};
}  // namespace

namespace VW
{
namespace io
{
namespace details
{
logger_impl::logger_impl(std::unique_ptr<log_sink> inner_stdout_logger, std::unique_ptr<log_sink> inner_stderr_logger)
    : stdout_log_sink(std::move(inner_stdout_logger)), stderr_log_sink(std::move(inner_stderr_logger))
{
}

class spdlog_log_sink : public log_sink
{
  std::unique_ptr<spdlog::logger> _spdlog_logger;

public:
  spdlog_log_sink(std::unique_ptr<spdlog::logger> spdlog_logger) : _spdlog_logger(std::move(spdlog_logger))
  {
    _spdlog_logger->set_pattern(DEFAULT_PATTERN);
    _spdlog_logger->set_level(spdlog::level::info);
  }
  virtual void info(const std::string& message) override { _spdlog_logger->info(message); }
  virtual void warn(const std::string& message) override { _spdlog_logger->warn(message); }
  virtual void error(const std::string& message) override { _spdlog_logger->error(message); }
  virtual void critical(const std::string& message) override { _spdlog_logger->critical(message); }

  virtual void set_level(log_level lvl) override
  {
    _spdlog_logger->set_level(static_cast<spdlog::level::level_enum>(lvl));
  }
};

}  // namespace details

logger::logger(std::shared_ptr<details::logger_impl> inner_logger) : _logger_impl(std::move(inner_logger)) {}

void logger::set_level(log_level lvl)
{
  _logger_impl->stderr_log_sink->set_level(lvl);
  _logger_impl->stdout_log_sink->set_level(lvl);
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

logger create_default_logger()
{
  auto stdout_spdlog_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto stdout_spdlog = std::unique_ptr<spdlog::logger>(new spdlog::logger("vowpal-stdout", stdout_spdlog_sink));
  auto stdout_sink = std::unique_ptr<details::log_sink>(new details::spdlog_log_sink(std::move(stdout_spdlog)));

  auto stderr_spdlog_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
  auto stderr_spdlog = std::unique_ptr<spdlog::logger>(new spdlog::logger("vowpal-stderr", stderr_spdlog_sink));
  auto stderr_sink = std::unique_ptr<details::log_sink>(new details::spdlog_log_sink(std::move(stderr_spdlog)));

  return logger(std::make_shared<details::logger_impl>(std::move(stdout_sink), std::move(stderr_sink)));
}

logger create_null_logger()
{
  auto stdout_spdlog_sink = std::make_shared<spdlog::sinks::null_sink_st>();
  auto stdout_spdlog = std::unique_ptr<spdlog::logger>(new spdlog::logger("vowpal-stdout", stdout_spdlog_sink));
  auto stdout_sink = std::unique_ptr<details::log_sink>(new details::spdlog_log_sink(std::move(stdout_spdlog)));

  auto stderr_spdlog_sink = std::make_shared<spdlog::sinks::null_sink_st>();
  auto stderr_spdlog = std::unique_ptr<spdlog::logger>(new spdlog::logger("vowpal-stderr", stderr_spdlog_sink));
  auto stderr_sink = std::unique_ptr<details::log_sink>(new details::spdlog_log_sink(std::move(stderr_spdlog)));

  return logger(std::make_shared<details::logger_impl>(std::move(stdout_sink), std::move(stderr_sink)));
}

logger create_custom_sink_logger(void* context, logger_output_func_t func)
{
  auto fptr_out_sink = std::make_shared<function_ptr_sink<std::mutex>>(context, func);
  auto stdout_spdlog = std::unique_ptr<spdlog::logger>(new spdlog::logger("vowpal-stdout", fptr_out_sink));
  auto stdout_sink = std::unique_ptr<details::log_sink>(new details::spdlog_log_sink(std::move(stdout_spdlog)));

  auto fptr_err_sink = std::make_shared<function_ptr_sink<std::mutex>>(context, func);
  auto stderr_spdlog = std::unique_ptr<spdlog::logger>(new spdlog::logger("vowpal-stderr", fptr_err_sink));
  auto stderr_sink = std::unique_ptr<details::log_sink>(new details::spdlog_log_sink(std::move(stderr_spdlog)));

  return logger(std::make_shared<details::logger_impl>(std::move(stdout_sink), std::move(stderr_sink)));
}

logger create_custom_sink_logger_legacy(void* context, logger_legacy_output_func_t func)
{
  auto fptr_out_sink = std::make_shared<function_ptr_legacy_sink<std::mutex>>(context, func);
  auto stdout_spdlog = std::unique_ptr<spdlog::logger>(new spdlog::logger("vowpal-stdout", fptr_out_sink));
  auto stdout_sink = std::unique_ptr<details::log_sink>(new details::spdlog_log_sink(std::move(stdout_spdlog)));

  auto fptr_err_sink = std::make_shared<function_ptr_legacy_sink<std::mutex>>(context, func);
  auto stderr_spdlog = std::unique_ptr<spdlog::logger>(new spdlog::logger("vowpal-stderr", fptr_err_sink));
  auto stderr_sink = std::unique_ptr<details::log_sink>(new details::spdlog_log_sink(std::move(stderr_spdlog)));

  return logger(std::make_shared<details::logger_impl>(std::move(stdout_sink), std::move(stderr_sink)));
}

}  // namespace io
}  // namespace VW
