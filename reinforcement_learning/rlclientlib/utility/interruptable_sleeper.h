#pragma once
#include <mutex>
#include <chrono>
#include <condition_variable>

namespace reinforcement_learning { namespace utility {
  // one use wakable sleeping class
  class interruptable_sleeper {
    std::condition_variable _cv;
    std::mutex _mutex;
    bool _interrupt = false;
  public:
    // waits until wake is called or the specified time passes
    template< class Rep, class Period >
    // returns true if timeout expired.  false if sleep was interrupted
    bool sleep(const std::chrono::duration<Rep, Period>& timeout_duration);
    // unblock sleeping thread
    void interrupt();
  };

  inline void interruptable_sleeper::interrupt() {
    {
      std::unique_lock <std::mutex> lock(_mutex);
      _interrupt = true;
    }
    _cv.notify_one();
  }

  /*
   * Sleep returns true if timeout expires and returns false if sleep was interrupted.
   */
  template <class Rep, class Period>
  bool interruptable_sleeper::sleep(const std::chrono::duration<Rep, Period>& timeout_duration) {
    try {
      std::unique_lock <std::mutex> lock(_mutex);
      return ! ( _cv.wait_for(lock, timeout_duration, [this]() { return _interrupt; }) );
    }
    catch(const std::exception &e) {
      return false;
    }
  }
}}