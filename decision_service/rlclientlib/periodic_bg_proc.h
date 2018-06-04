#pragma once
#include <algorithm>
#include <thread>
#include "err_constants.h"
#include "error_callback_fn.h"
#include "api_status.h"
#include "interruptable_sleeper.h"

namespace reinforcement_learning {
  class error_callback_fn;
}

namespace reinforcement_learning { namespace utility {

  template<typename BGProc>
  class periodic_bg_proc {
  public:
    periodic_bg_proc(const int interval_ms, BGProc& bgproc, error_callback_fn* perror_cb = nullptr);
    int init(api_status* status = nullptr);
    void stop();
    ~periodic_bg_proc();

    periodic_bg_proc(const periodic_bg_proc&) = delete;
    periodic_bg_proc(periodic_bg_proc&&) = delete;
    periodic_bg_proc& operator=(const periodic_bg_proc&) = delete;
    periodic_bg_proc& operator=(periodic_bg_proc&&) = delete;
  private:
    void time_loop();

  private:
    bool _thread_is_running;
    BGProc _proc;
    std::thread _background_thread;    
    int _interval_ms;
    error_callback_fn* _perror_cb;
    interruptable_sleeper _sleeper;
  };

  template <typename BGProc>
  periodic_bg_proc<BGProc>::periodic_bg_proc(const int interval_ms, BGProc& bgproc, error_callback_fn* perror_cb)
    : _thread_is_running { false }, _proc(std::move(bgproc)), _interval_ms { interval_ms }, _perror_cb(perror_cb) 
  {}

  template <typename BGProc>
  int periodic_bg_proc<BGProc>::init(api_status* status) {

    if ( !_thread_is_running ) {
      try {
        _thread_is_running = true;
        _background_thread = std::thread(&periodic_bg_proc::time_loop, this);
      }
      catch ( const std::exception& e ) {
        _thread_is_running = false;
        return report_error(status, error_code::background_thread_start,
          "Unable to start background thread to retrieve models.",
          e.what());
      }
    }
    return error_code::success;
  }

  template <typename BGProc>
  void periodic_bg_proc<BGProc>::stop() {
    if ( _thread_is_running ) {
      _thread_is_running = false;
      _sleeper.interrupt();
      _background_thread.join();
    }
  }

  template <typename BGProc>
  periodic_bg_proc<BGProc>::~periodic_bg_proc() {
    stop();
  }

  template <typename BGProc>
  void periodic_bg_proc<BGProc>::time_loop() {
    while ( _thread_is_running ) {
      api_status status;
      // Run the background task once
      if ( _proc.run_once(&status) != error_code::success ) {
        ERROR_CALLBACK(_perror_cb, status);
      }
      // Cancelable sleep for interval
      if ( !_thread_is_running || !_sleeper.sleep(std::chrono::milliseconds(_interval_ms)) )
        return;
    }
  }

}}
