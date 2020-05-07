#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "guard.h"

struct non_copyable_struct
{
  int _value;
  explicit non_copyable_struct(int value) : _value(value) {}

  non_copyable_struct(const non_copyable_struct&) = delete;
  non_copyable_struct& operator=(const non_copyable_struct&) = delete;
  non_copyable_struct(non_copyable_struct&& other) = default;
  non_copyable_struct& operator=(non_copyable_struct&& other) = default;
};


BOOST_AUTO_TEST_CASE(swap_guard_execute_on_scope_end)
{
  int original_location = 1;
  int location_to_swap = 99999;

  {
    BOOST_CHECK_EQUAL(original_location, 1);
    BOOST_CHECK_EQUAL(location_to_swap, 99999);
    auto guard = VW::swap_guard(original_location, location_to_swap);
    BOOST_CHECK_EQUAL(original_location, 99999);
    BOOST_CHECK_EQUAL(location_to_swap, 1);
  }
  BOOST_CHECK_EQUAL(original_location, 1);
  BOOST_CHECK_EQUAL(location_to_swap, 99999);
}

BOOST_AUTO_TEST_CASE(swap_guard_execute_on_scope_end_no_copy)
{
  non_copyable_struct original_location(1);
  non_copyable_struct location_to_swap(99999);

  {
    BOOST_CHECK_EQUAL(original_location._value, 1);
    BOOST_CHECK_EQUAL(location_to_swap._value, 99999);
    auto guard = VW::swap_guard(original_location, location_to_swap);
    BOOST_CHECK_EQUAL(original_location._value, 99999);
    BOOST_CHECK_EQUAL(location_to_swap._value, 1);
  }
  BOOST_CHECK_EQUAL(original_location._value, 1);
  BOOST_CHECK_EQUAL(location_to_swap._value, 99999);
}

BOOST_AUTO_TEST_CASE(swap_guard_cancel)
{
  int original_location = 1;
  int location_to_swap = 99999;

  {
    BOOST_CHECK_EQUAL(original_location, 1);
    BOOST_CHECK_EQUAL(location_to_swap, 99999);
    auto guard = VW::swap_guard(original_location, location_to_swap);
    BOOST_CHECK_EQUAL(original_location, 99999);
    BOOST_CHECK_EQUAL(location_to_swap, 1);
    guard.cancel();
  }
  BOOST_CHECK_EQUAL(original_location, 99999);
  BOOST_CHECK_EQUAL(location_to_swap, 1);
}

BOOST_AUTO_TEST_CASE(swap_guard_explicit_force_swap)
{
  int original_location = 1;
  int location_to_swap = 99999;

  {
    BOOST_CHECK_EQUAL(original_location, 1);
    BOOST_CHECK_EQUAL(location_to_swap, 99999);
    auto guard = VW::swap_guard(original_location, location_to_swap);
    BOOST_CHECK_EQUAL(original_location, 99999);
    BOOST_CHECK_EQUAL(location_to_swap, 1);
    BOOST_CHECK_EQUAL(guard.do_swap(), true);
    BOOST_CHECK_EQUAL(original_location, 1);
    BOOST_CHECK_EQUAL(location_to_swap, 99999);
    BOOST_CHECK_EQUAL(guard.do_swap(), false);
    BOOST_CHECK_EQUAL(original_location, 1);
    BOOST_CHECK_EQUAL(location_to_swap, 99999);
  }
  BOOST_CHECK_EQUAL(original_location, 1);
  BOOST_CHECK_EQUAL(location_to_swap, 99999);
}

BOOST_AUTO_TEST_CASE(swap_guard_execute_on_scope_end_swap_pointers)
{
  int* original_location = new int;
  *original_location = 1;
  int* location_to_swap = new int;
  *location_to_swap = 99999;

  int* original_location_pre_swap = original_location;
  int* location_to_swap_pre_swap = location_to_swap;

  {
    BOOST_CHECK_EQUAL(*original_location, 1);
    BOOST_CHECK_EQUAL(*location_to_swap, 99999);
    BOOST_CHECK_EQUAL(original_location, original_location_pre_swap);
    BOOST_CHECK_EQUAL(location_to_swap, location_to_swap_pre_swap);
    auto guard = VW::swap_guard(original_location, location_to_swap);
    BOOST_CHECK_EQUAL(*original_location, 99999);
    BOOST_CHECK_EQUAL(*location_to_swap, 1);
    BOOST_CHECK_EQUAL(original_location, location_to_swap_pre_swap);
    BOOST_CHECK_EQUAL(location_to_swap, original_location_pre_swap);
  }
  BOOST_CHECK_EQUAL(*original_location, 1);
  BOOST_CHECK_EQUAL(*location_to_swap, 99999);
  BOOST_CHECK_EQUAL(original_location, original_location_pre_swap);
  BOOST_CHECK_EQUAL(location_to_swap, location_to_swap_pre_swap);

  delete original_location;
  delete location_to_swap;
}

BOOST_AUTO_TEST_CASE(swap_guard_execute_temp_value)
{
  int original_location = 1;

  {
    BOOST_CHECK_EQUAL(original_location, 1);
    auto guard = VW::swap_guard(original_location, 9999);
    BOOST_CHECK_EQUAL(original_location, 9999);
  }
  BOOST_CHECK_EQUAL(original_location, 1);
}

BOOST_AUTO_TEST_CASE(swap_guard_execute_temp_value_no_copy)
{
  non_copyable_struct original_location(1);

  {
    BOOST_CHECK_EQUAL(original_location._value, 1);
    auto guard = VW::swap_guard(original_location, non_copyable_struct(9999));
    BOOST_CHECK_EQUAL(original_location._value, 9999);
  }
  BOOST_CHECK_EQUAL(original_location._value, 1);
}