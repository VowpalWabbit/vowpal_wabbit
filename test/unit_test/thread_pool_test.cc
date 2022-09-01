// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/thread_pool.h"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <cstring>
#include <string>
#include <vector>

// fills the inner vector at index vector_index with the size_of_inner_vector numbers of vector_index's
auto fill_one_vector =
    [](size_t vector_index, std::vector<std::vector<size_t>>& vector_of_vectors, size_t size_of_inner_vector)
{
  vector_of_vectors[vector_index].resize(size_of_inner_vector, vector_index);
  return size_of_inner_vector;
};

std::vector<std::vector<size_t>> get_expected_vector(size_t how_many_inner_vectors, size_t size_of_inner_vectors)
{
  std::vector<std::vector<size_t>> test_vector;
  for (size_t i = 0; i < how_many_inner_vectors; i++)
  {
    std::vector<size_t> inner_vector(size_of_inner_vectors, i);
    test_vector.push_back(inner_vector);
  }

  return test_vector;
}

void compare_expected_and_result_vectors(
    const std::vector<std::vector<size_t>>& expected_vector, const std::vector<std::vector<size_t>>& result_vector)
{
  BOOST_CHECK_EQUAL(expected_vector.size(), result_vector.size());

  for (size_t i = 0; i < result_vector.size(); i++)
  {
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result_vector[i].begin(), result_vector[i].end(), expected_vector[i].begin(), expected_vector[i].end());
  }
}

BOOST_AUTO_TEST_SUITE(test_suite_thread_pool)

BOOST_AUTO_TEST_CASE(test_thread_pool_with_zero_threads)
{
  size_t outer_vector_size = 10;
  size_t inner_vectors_size = 10;

  std::vector<std::vector<size_t>> vector_of_vectors;
  vector_of_vectors.resize(outer_vector_size);

  VW::thread_pool thread_pool_(0);
  std::vector<std::future<size_t>> fts;

  for (size_t i = 0; i < vector_of_vectors.size(); i++)
  {
    fts.emplace_back(thread_pool_.submit(fill_one_vector, i, std::ref(vector_of_vectors), inner_vectors_size));
  }

  for (auto& ft : fts)
  {
    size_t returned = ft.get();
    BOOST_CHECK_EQUAL(inner_vectors_size, returned);
  }

  compare_expected_and_result_vectors(get_expected_vector(inner_vectors_size, outer_vector_size), vector_of_vectors);
}

BOOST_AUTO_TEST_CASE(test_thread_pool_with_more_threads_than_tasks)
{
  size_t outer_vector_size = 10;
  size_t inner_vectors_size = 10;

  std::vector<std::vector<size_t>> vector_of_vectors;
  vector_of_vectors.resize(outer_vector_size);

  VW::thread_pool thread_pool_(20);
  std::vector<std::future<size_t>> fts;

  for (size_t i = 0; i < vector_of_vectors.size(); i++)
  {
    fts.emplace_back(thread_pool_.submit(fill_one_vector, i, std::ref(vector_of_vectors), inner_vectors_size));
  }

  for (auto& ft : fts)
  {
    size_t returned = ft.get();
    BOOST_CHECK_EQUAL(inner_vectors_size, returned);
  }

  compare_expected_and_result_vectors(get_expected_vector(inner_vectors_size, outer_vector_size), vector_of_vectors);
}

BOOST_AUTO_TEST_CASE(test_thread_pool_with_less_threads_than_tasks)
{
  size_t outer_vector_size = 10;
  size_t inner_vectors_size = 10;

  std::vector<std::vector<size_t>> vector_of_vectors;
  vector_of_vectors.resize(outer_vector_size);

  VW::thread_pool thread_pool_(5);
  std::vector<std::future<size_t>> fts;

  for (size_t i = 0; i < vector_of_vectors.size(); i++)
  {
    fts.emplace_back(thread_pool_.submit(fill_one_vector, i, std::ref(vector_of_vectors), inner_vectors_size));
  }

  for (auto& ft : fts)
  {
    size_t returned = ft.get();
    BOOST_CHECK_EQUAL(inner_vectors_size, returned);
  }

  compare_expected_and_result_vectors(get_expected_vector(inner_vectors_size, outer_vector_size), vector_of_vectors);
}

BOOST_AUTO_TEST_SUITE_END()