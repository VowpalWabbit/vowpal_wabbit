// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/guard.h"

#include "vw/core/memory.h"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>

class non_copyable_class
{
public:
  int value;
  explicit non_copyable_class(int value) : value(value) {}

  non_copyable_class(const non_copyable_class&) = delete;
  non_copyable_class& operator=(const non_copyable_class&) = delete;
  non_copyable_class(non_copyable_class&& other) = default;
  non_copyable_class& operator=(non_copyable_class&& other) = default;
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
  non_copyable_class original_location(1);
  non_copyable_class location_to_swap(99999);

  {
    BOOST_CHECK_EQUAL(original_location.value, 1);
    BOOST_CHECK_EQUAL(location_to_swap.value, 99999);
    auto guard = VW::swap_guard(original_location, location_to_swap);
    BOOST_CHECK_EQUAL(original_location.value, 99999);
    BOOST_CHECK_EQUAL(location_to_swap.value, 1);
  }
  BOOST_CHECK_EQUAL(original_location.value, 1);
  BOOST_CHECK_EQUAL(location_to_swap.value, 99999);
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
  non_copyable_class original_location(1);

  {
    BOOST_CHECK_EQUAL(original_location.value, 1);
    auto guard = VW::swap_guard(original_location, non_copyable_class(9999));
    BOOST_CHECK_EQUAL(original_location.value, 9999);
  }
  BOOST_CHECK_EQUAL(original_location.value, 1);
}

BOOST_AUTO_TEST_CASE(swap_guard_unique_ptr)
{
  std::unique_ptr<int> original_location = VW::make_unique<int>(1);

  {
    std::unique_ptr<int> inner_location = VW::make_unique<int>(9999);
    BOOST_CHECK_EQUAL(*inner_location, 9999);
    BOOST_CHECK_EQUAL(*original_location, 1);
    auto guard = VW::swap_guard(original_location, inner_location);
    BOOST_CHECK_EQUAL(*inner_location, 1);
    BOOST_CHECK_EQUAL(*original_location, 9999);
  }
  BOOST_CHECK_EQUAL(*original_location, 1);
}

BOOST_AUTO_TEST_CASE(stash_guard_execute_on_scope_end)
{
  int target_location = 999;
  {
    BOOST_CHECK_EQUAL(target_location, 999);
    auto guard = VW::stash_guard(target_location);
    BOOST_CHECK_EQUAL(target_location, 0);
  }
  BOOST_CHECK_EQUAL(target_location, 999);
}

class struct_with_non_trivial_ctor
{
public:
  int value;
  struct_with_non_trivial_ctor() : value(123) {}
};

BOOST_AUTO_TEST_CASE(stash_guard_used_default_ctor)
{
  struct_with_non_trivial_ctor target_location;
  BOOST_CHECK_EQUAL(target_location.value, 123);

  target_location.value = 456;
  {
    BOOST_CHECK_EQUAL(target_location.value, 456);
    auto guard = VW::stash_guard(target_location);
    BOOST_CHECK_EQUAL(target_location.value, 123);
  }
  BOOST_CHECK_EQUAL(target_location.value, 456);
}
