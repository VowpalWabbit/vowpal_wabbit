#pragma once
#include <thread>
#include <map>
#include <mutex>

#include "api_status.h"
#include "error_callback_fn.h"

#include <atomic>
#include "interruptable_sleeper.h"

namespace reinforcement_learning {
  class i_trace;
  namespace utility {


    class watchdog {
    public:
      explicit watchdog(error_callback_fn* error_callback = nullptr);
      ~watchdog();

      void register_thread(std::thread::id const& thread_id, std::string const& thread_name, long long const timeout);
      void unregister_thread(std::thread::id const& thread_id);
      void check_in(std::thread::id const& thread_id);

      int set_trace_log(i_trace* trace_logger);
      int start(api_status* status);
      void stop();
      void loop();

      bool has_background_error_been_reported() const;
      void set_unhandled_background_error(bool const value);

      watchdog(watchdog const& other) = delete;
      watchdog(watchdog&& other) = delete;
      watchdog& operator=(watchdog const& other) = delete;
      watchdog& operator=(watchdog&& other) = delete;
    private:
      using clock_t = std::chrono::system_clock;

      struct thread_info {
        std::thread::id thread_id;
        std::string thread_name;
        //! Last time the watchdog verified this thread was still active.
        clock_t::time_point last_verify_time;
        //! Last time the thread checked in.
        clock_t::time_point last_check_in_time;
        std::chrono::milliseconds timeout;
      };

      std::mutex _watchdog_mutex;
      interruptable_sleeper _sleeper;
      std::thread _watchdog_thread;
      std::atomic<bool> _running{ false };

      std::chrono::milliseconds _timeout_in_ms{10000};
      std::map<std::thread::id, thread_info> _thread_infos;

      error_callback_fn* _error_callback;
      std::atomic<bool> _unhandled_background_error_occurred{false};
      i_trace* _trace_logger;
    };
  }
}
