#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include "utility/watchdog.h"
#include "str_util.h"

#include <atomic>

using namespace reinforcement_learning;

constexpr int timeout = 100;
constexpr int fail_timeout = timeout * 2;
constexpr int safe_timeout = timeout / 4;

BOOST_AUTO_TEST_CASE(watchdog_fail_after_registration) {
  utility::watchdog watchdog(nullptr);
  watchdog.start(nullptr);

  // Verify it begins not in an error state.
  BOOST_CHECK_EQUAL(watchdog.has_background_error_been_reported(), false);

  std::this_thread::sleep_for(std::chrono::milliseconds(fail_timeout));

  // Verify after waiting with no registered threads there is still no error.
  BOOST_CHECK_EQUAL(watchdog.has_background_error_been_reported(), false);

  watchdog.register_thread(std::this_thread::get_id(), "Test thread 1", timeout);

  std::this_thread::sleep_for(std::chrono::milliseconds(fail_timeout));
  BOOST_CHECK_EQUAL(watchdog.has_background_error_been_reported(), true);
}

BOOST_AUTO_TEST_CASE(watchdog_unregister) {
  utility::watchdog watchdog(nullptr);
  watchdog.start(nullptr);

  watchdog.register_thread(std::this_thread::get_id(), "Test thread 1", timeout);
  BOOST_CHECK_EQUAL(watchdog.has_background_error_been_reported(), false);
  watchdog.unregister_thread(std::this_thread::get_id());
  std::this_thread::sleep_for(std::chrono::milliseconds(fail_timeout));
  BOOST_CHECK_EQUAL(watchdog.has_background_error_been_reported(), false);
}

BOOST_AUTO_TEST_CASE(watchdog_fail_after_several_iterations) {
  utility::watchdog watchdog(nullptr);
  watchdog.start(nullptr);

  watchdog.register_thread(std::this_thread::get_id(), "Test thread 1", timeout);
  BOOST_CHECK_EQUAL(watchdog.has_background_error_been_reported(), false);

  for(auto i = 0; i < 5; i++) {
    watchdog.check_in(std::this_thread::get_id());
    BOOST_CHECK_EQUAL(watchdog.has_background_error_been_reported(), false);
    std::this_thread::sleep_for(std::chrono::milliseconds(safe_timeout));
  }

  BOOST_CHECK_EQUAL(watchdog.has_background_error_been_reported(), false);
  std::this_thread::sleep_for(std::chrono::milliseconds(fail_timeout));
  BOOST_CHECK_EQUAL(watchdog.has_background_error_been_reported(), true);
}

BOOST_AUTO_TEST_CASE(watchdog_report_with_error_handler) {
  std::atomic<int> atomic_counter;
  atomic_counter.store(0);

  auto const error_fn = [](const api_status&, void* arg) {
    auto counter = static_cast<std::atomic<int>*>(arg);
    counter->fetch_add(1);
  };

  error_callback_fn err_func(error_fn, &atomic_counter);

  utility::watchdog watchdog(&err_func);
  watchdog.start(nullptr);

  watchdog.register_thread(std::this_thread::get_id(), "Test thread 1", timeout);

  std::this_thread::sleep_for(std::chrono::milliseconds(fail_timeout));
  BOOST_CHECK_GT(atomic_counter.load(), 0);
}

BOOST_AUTO_TEST_CASE(watchdog_multiple_threads) {
  utility::watchdog watchdog(nullptr);
  watchdog.start(nullptr);

  std::vector<std::thread> threads;
  const auto num_threads = 100;
  const auto num_iterations = 100;
  threads.reserve(num_threads);
  for(auto i = 0 ; i < num_threads; i++) {
    threads.emplace_back([&, i]() {
      watchdog.register_thread(std::this_thread::get_id(), utility::concat("Test thread ", i), timeout);

      for (auto j = 0; j < num_iterations; j++) {
        watchdog.check_in(std::this_thread::get_id());
        std::this_thread::sleep_for(std::chrono::milliseconds(safe_timeout));
      }

      watchdog.unregister_thread(std::this_thread::get_id());
    });
  }

  BOOST_CHECK_EQUAL(watchdog.has_background_error_been_reported(), false);

  for(auto& t : threads) {
    if(t.joinable()) {
      t.join();
    }
  }
}