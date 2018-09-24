#include "watchdog.h"

#include "str_util.h"
#include <utility>
#include <vector>
#include <algorithm>
#include "trace_logger.h"

using namespace reinforcement_learning;
using namespace reinforcement_learning::utility;

watchdog::watchdog(error_callback_fn* error_callback)
  : _error_callback(error_callback), _trace_logger(nullptr) {}

watchdog::~watchdog() {
  stop();
}

void watchdog::register_thread(std::thread::id const& thread_id, std::string const& thread_name, long long const timeout) {
  std::lock_guard<std::mutex> lock(_watchdog_mutex);

  // Watchdog timeout should reflect the thread with the tightest time requirement.
  auto const long_long_duration = std::chrono::duration_cast<std::chrono::duration<long long>>(_timeout_in_ms);
  _timeout_in_ms = std::chrono::milliseconds(std::min(timeout, long_long_duration.count()));

  _thread_infos.emplace(thread_id, thread_info{
    thread_id,
    thread_name,
    clock_t::now(),
    clock_t::now(),
    std::chrono::milliseconds{ timeout }
  });

  // Wake the sleeper so that the new timeout can take effect.
  _sleeper.interrupt();
}

void watchdog::unregister_thread(std::thread::id const& thread_id) {
  std::lock_guard<std::mutex> lock(_watchdog_mutex);

  auto const iterator = _thread_infos.find(thread_id);
  if (iterator != _thread_infos.end()) {
    _thread_infos.erase(iterator);
  }
}

void watchdog::check_in(std::thread::id const& thread_id) {
  std::lock_guard<std::mutex> lock(_watchdog_mutex);

  auto const it = _thread_infos.find(thread_id);
  if (it == _thread_infos.end()) {
    throw std::runtime_error(concat("Thread ", thread_id, " must be registered before being used."));
  }

  auto& thread_info = it->second;
  thread_info.last_check_in_time = clock_t::now();
}

void watchdog::set_trace_log(i_trace* trace_logger) { _trace_logger = trace_logger; }

int watchdog::start(api_status* status) {
  auto expected_value = false;
  if(_running.compare_exchange_strong(expected_value, true)) {
    try {
      _watchdog_thread = std::thread(&watchdog::loop, this);
    }
    catch (const std::exception& e) {
      RETURN_ERROR_LS(_trace_logger, status, background_thread_start) << " (watchdog)" << e.what();
    }
  }

  return error_code::success;
}

void watchdog::stop() {
  auto expected_value = true;
  if (_running.compare_exchange_strong(expected_value, false)) {
    _sleeper.interrupt();

    if (_watchdog_thread.joinable()) {
      _watchdog_thread.join();
    }
  }
}

void watchdog::loop() {
  while (_running.load()) {
    // If enough time has passed to be beyond the timeout, check the state of all registered threads.
    // If a thread hasn't checking in during this timeout period it is assumed to be unresponsive and an error is generated.
    std::vector<std::string> failed_thread_names;

    {
      std::unique_lock<std::mutex> lock(_watchdog_mutex);
      for (auto& kv : _thread_infos) {
        auto& thread_info = kv.second;

        thread_info.last_verify_time = clock_t::now();

        if(thread_info.last_verify_time - thread_info.last_check_in_time > thread_info.timeout) {
          if (_error_callback) {
            failed_thread_names.push_back(thread_info.thread_name);
          }
          else {
            set_unhandled_background_error(true);
          }
        }
      }
    }

    api_status status;
    for (auto const& failed_thread_name : failed_thread_names) {
      auto message = concat(error_code::thread_unresponsive_timeout, ", ", failed_thread_name, " is unresponsive.");
      TRACE_LOG(_trace_logger, message);
      api_status::try_update(&status, error_code::thread_unresponsive_timeout, message.c_str());
      _error_callback->report_error(status);
    }

    _sleeper.sleep(std::chrono::milliseconds{ _timeout_in_ms });
  }
}

bool watchdog::has_background_error_been_reported() const {
  return _unhandled_background_error_occurred.load();
}

void watchdog::set_unhandled_background_error(bool const value) {
  _unhandled_background_error_occurred.store(value);
}
