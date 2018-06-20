#pragma once
#include <thread>
#include "err_constants.h"
#include "error_callback_fn.h"
#include "api_status.h"
#include "interruptable_sleeper.h"

namespace reinforcement_learning {
  class error_callback_fn;
}

namespace reinforcement_learning { namespace utility {

  template<typename BgProc>
  class periodic_background_proc {
  public:
    // Construction and init
    explicit periodic_background_proc(const int interval_ms, error_callback_fn* perror_cb = nullptr);
    int init(BgProc* bgproc, api_status* status = nullptr);

    // Shutdown and Destructor
    ~periodic_background_proc();
    void stop();

    // Cannot copy, assign
    periodic_background_proc(const periodic_background_proc&) = delete;
    periodic_background_proc(periodic_background_proc&&) = delete;
    periodic_background_proc& operator=(const periodic_background_proc&) = delete;
    periodic_background_proc& operator=(periodic_background_proc&&) = delete;

  private:
    // Implementation methods
    void time_loop();

  private:
    // Internal state
    bool _thread_is_running;
    int _interval_ms;
    std::thread _background_thread;
    interruptable_sleeper _sleeper;

    // Pointers not owned by this class.  Lifetime is managed externally
    BgProc* _proc;
    error_callback_fn* _perror_cb;
  };

  template <typename BgProc>
  periodic_background_proc<BgProc>::periodic_background_proc(const int interval_ms, error_callback_fn* perror_cb)
    : _thread_is_running { false }, _interval_ms { interval_ms }, 
      _proc(nullptr), _perror_cb(perror_cb) 
  {}

  template <typename BgProc>
  int periodic_background_proc<BgProc>::init(BgProc* bgproc, api_status* status) {
    if( bgproc == nullptr )
      return report_error(status, error_code::invalid_argument, "Invalid BGProc");

    _proc = bgproc;

    if ( !_thread_is_running ) {
      try {
        _thread_is_running = true;
        _background_thread = std::thread(&periodic_background_proc::time_loop, this);
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

  template <typename BgProc>
  void periodic_background_proc<BgProc>::stop() {
    if ( _thread_is_running ) {
      _thread_is_running = false;
      _sleeper.interrupt();
      _background_thread.join();
    }
  }

  template <typename BGProc>
  periodic_background_proc<BGProc>::~periodic_background_proc() {
    stop();
  }

  template <typename BGProc>
  void periodic_background_proc<BGProc>::time_loop() {
    while ( _thread_is_running ) {
      api_status status;
      
      // Run the background task once
      if ( _proc.run_iteration(&status) != error_code::success ) {
        ERROR_CALLBACK(_perror_cb, status);
      }
      // Cancelable sleep for interval
      if ( !_thread_is_running || !_sleeper.sleep(std::chrono::milliseconds(_interval_ms)) )
        return;
    }
  }

}}
