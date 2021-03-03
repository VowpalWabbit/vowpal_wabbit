#ifndef STATIC_LINK_VW
#  define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <cstddef>
#include <algorithm>

#include "v_array.h"

BOOST_AUTO_TEST_CASE(v_array_size_is_const)
{
  v_array<int> list;
  list.push_back(1);
  list.push_back(2);

  const auto& const_list = list;
  BOOST_CHECK_EQUAL(std::size_t(2), const_list.size());
}

BOOST_AUTO_TEST_CASE(v_array_empty_is_const)
{
  const v_array<int> list;
  BOOST_CHECK_EQUAL(true, list.empty());
}

BOOST_AUTO_TEST_CASE(v_array_dereference)
{
  v_array<int> list;
  list.push_back(1);
  list.push_back(2);

  BOOST_CHECK_EQUAL(std::size_t(2), list[1]);
}

BOOST_AUTO_TEST_CASE(v_array_clear)
{
  v_array<int> list;
  list.push_back(1);
  list.push_back(2);
  BOOST_CHECK_EQUAL(std::size_t(2), list.size());
  list.clear();
  BOOST_CHECK_EQUAL(std::size_t(0), list.size());
}

BOOST_AUTO_TEST_CASE(v_array_copy)
{
  v_array<int> list;
  list.push_back(1);
  list.push_back(2);
  BOOST_CHECK_EQUAL(std::size_t(2), list.size());

  v_array<int> list2 = list;
  BOOST_CHECK_EQUAL(std::size_t(2), list2.size());

  v_array<int> list3(list);
  BOOST_CHECK_EQUAL(std::size_t(2), list3.size());
}

BOOST_AUTO_TEST_CASE(v_array_move)
{
  v_array<int> list;
  list.push_back(1);
  list.push_back(2);
  BOOST_CHECK_EQUAL(std::size_t(2), list.size());

  v_array<int> list2 = std::move(list);
  BOOST_CHECK_EQUAL(std::size_t(2), list2.size());

  v_array<int> list3(std::move(list2));
  BOOST_CHECK_EQUAL(std::size_t(2), list3.size());
}

BOOST_AUTO_TEST_CASE(v_array_pop_back)
{
  v_array<int> list;
  list.push_back(1);
  list.push_back(2);
  BOOST_CHECK_EQUAL(std::size_t(2), list.size());
  list.pop_back();
  BOOST_CHECK_EQUAL(std::size_t(1), list.size());
  list.pop_back();
  BOOST_CHECK_EQUAL(std::size_t(0), list.size());
  BOOST_CHECK_EQUAL(true, list.empty());
}

BOOST_AUTO_TEST_CASE(v_array_find_exists)
{
  v_array<int> list;
  list.push_back(1);
  list.push_back(2);
  list.push_back(6);
  list.push_back(4);

  auto it = std::find(list.begin(), list.end(), 2);
  BOOST_CHECK_NE(it, list.end());
  BOOST_CHECK_EQUAL(it, list.begin() + 1);
}

BOOST_AUTO_TEST_CASE(v_array_find_not_exists)
{
  v_array<int> list;
  list.push_back(1);
  list.push_back(2);
  list.push_back(6);
  list.push_back(4);

  auto it = std::find(list.begin(), list.end(), 11);
  BOOST_CHECK_EQUAL(it, list.end());
}

BOOST_AUTO_TEST_CASE(v_array_back)
{
  v_array<int> list;
  list.push_back(1);
  list.push_back(2);
  BOOST_CHECK_EQUAL(2, list.back());
}