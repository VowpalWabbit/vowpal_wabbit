#pragma once
#include "err_constants.h"
#include "error_callback_fn.h"
#include "api_status.h"
#include "interruptable_sleeper.h"

#include "utility/watchdog.h"

#include <thread>
#include <string>

namespace reinforcement_learning {
  class error_callback_fn;
}

namespace reinforcement_learning {
  namespace utility {

    constexpr float timeout_grace_multiplier_c = 3.f;

    template <typename BgProc>
    class periodic_background_proc {
    public:
      // Construction and init
      explicit periodic_background_proc(const int interval_ms, utility::watchdog& watchdog,
        std::string const& proc_name, error_callback_fn* perror_cb = nullptr);
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

      watchdog& _watchdog;
      std::string _proc_name;

      // Pointers not owned by this class.  Lifetime is managed externally
      BgProc* _proc;
      error_callback_fn* _perror_cb;
    };

    template <typename BgProc>
    periodic_background_proc<BgProc>::periodic_background_proc(const int interval_ms, watchdog& watchdog,
      std::string const& proc_name, error_callback_fn* perror_cb)
      : _thread_is_running {false},
        _interval_ms{interval_ms},
        _watchdog(watchdog),
        _proc_name(proc_name),
        _proc(nullptr),
        _perror_cb(perror_cb)
    {}

    template <typename BgProc>
    int periodic_background_proc<BgProc>::init(BgProc* bgproc, api_status* status) {
      if (bgproc == nullptr) {
        RETURN_ERROR_LS(nullptr, status, invalid_argument) << " (BGProc)";
      }

      _proc = bgproc;

      if (!_thread_is_running) {
        try {
          _thread_is_running = true;
          _background_thread = std::thread(&periodic_background_proc::time_loop, this);
        }
        catch (const std::exception& e) {
          _thread_is_running = false;
          RETURN_ERROR_LS(nullptr, status, background_thread_start) << " (retrieve models)" << e.what();
        }
      }
      return error_code::success;
    }

    template <typename BgProc>
    void periodic_background_proc<BgProc>::stop() {
      if (_thread_is_running) {
        _thread_is_running = false;
        _sleeper.interrupt();

        auto const background_thread_id = _background_thread.get_id();
        _background_thread.join();
        // Unregister the thread from the watchdog after it has completely finished execution.
        _watchdog.unregister_thread(background_thread_id);
      }
    }

    template <typename BGProc>
    periodic_background_proc<BGProc>::~periodic_background_proc() {
      stop();
    }

    template <typename BGProc>
    void periodic_background_proc<BGProc>::time_loop() {
      // The first action of the thread should be registering itself with the watchdog.
      _watchdog.register_thread(std::this_thread::get_id(), _proc_name, static_cast<long long>(_interval_ms * timeout_grace_multiplier_c));

      while (_thread_is_running) {
        api_status status;

        // Check in to the watchdog to report this thread is still alive.
        _watchdog.check_in(std::this_thread::get_id());

        // Run the background task once
        if (_proc->run_iteration(&status) != error_code::success) {
          ERROR_CALLBACK(_perror_cb, status);
        }
        // Cancelable sleep for interval
        if (!_thread_is_running || !_sleeper.sleep(std::chrono::milliseconds(_interval_ms)))
          return;
      }
    }
  }
}
