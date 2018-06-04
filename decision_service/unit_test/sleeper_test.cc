#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>
#include "interruptable_sleeper.h"

namespace u = reinforcement_learning::utility;

BOOST_AUTO_TEST_CASE(sleeper_interrupt) {
  u::interruptable_sleeper sleeper;
  std::thread t([&]() {
    // test interruption
    const auto start = std::chrono::system_clock::now();
    sleeper.sleep(std::chrono::milliseconds(5000));
    const auto stop = std::chrono::system_clock::now();
    const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>( stop - start );
    BOOST_CHECK(diff <= std::chrono::milliseconds(100));
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  sleeper.interrupt();
  t.join();
}

BOOST_AUTO_TEST_CASE(sleeper_sleep) {
  u::interruptable_sleeper sleeper;
  std::thread t([&]() {
    // test interruption
    const auto start = std::chrono::system_clock::now();
    sleeper.sleep(std::chrono::milliseconds(100));
    const auto stop = std::chrono::system_clock::now();
    const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>( stop - start );
    BOOST_CHECK(diff >= std::chrono::milliseconds(80));
  });
  t.join();
}