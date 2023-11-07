// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/v_array.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>

TEST(VArray, SizeIsConst)
{
  VW::v_array<int> list;
  list.push_back(1);
  list.push_back(2);

  const auto& const_list = list;
  EXPECT_EQ(std::size_t(2), const_list.size());
}

TEST(VArray, EmptyIsConst)
{
  const VW::v_array<int> list;
  EXPECT_EQ(true, list.empty());
}

TEST(VArray, Dereference)
{
  VW::v_array<int> list;
  list.push_back(1);
  list.push_back(2);

  EXPECT_EQ(std::size_t(2), list[1]);
}

TEST(VArray, Clear)
{
  VW::v_array<int> list;
  list.push_back(1);
  list.push_back(2);
  EXPECT_EQ(std::size_t(2), list.size());
  list.clear();
  EXPECT_EQ(std::size_t(0), list.size());
}

TEST(VArray, Copy)
{
  VW::v_array<int> list;
  list.push_back(1);
  list.push_back(2);
  EXPECT_EQ(std::size_t(2), list.size());

  VW::v_array<int> list2 = list;
  EXPECT_EQ(std::size_t(2), list2.size());

  VW::v_array<int> list3(list);
  EXPECT_EQ(std::size_t(2), list3.size());
}

TEST(VArray, Move)
{
  VW::v_array<int> list;
  list.push_back(1);
  list.push_back(2);
  EXPECT_EQ(std::size_t(2), list.size());

  VW::v_array<int> list2 = std::move(list);
  EXPECT_EQ(std::size_t(2), list2.size());

  VW::v_array<int> list3(std::move(list2));
  EXPECT_EQ(std::size_t(2), list3.size());
}

TEST(VArray, PopBack)
{
  VW::v_array<int> list;
  list.push_back(1);
  list.push_back(2);
  EXPECT_EQ(std::size_t(2), list.size());
  list.pop_back();
  EXPECT_EQ(std::size_t(1), list.size());
  list.pop_back();
  EXPECT_EQ(std::size_t(0), list.size());
  EXPECT_EQ(true, list.empty());
}

TEST(VArray, FindExists)
{
  VW::v_array<int> list;
  list.push_back(1);
  list.push_back(2);
  list.push_back(6);
  list.push_back(4);

  auto it = std::find(list.begin(), list.end(), 2);
  EXPECT_NE(it, list.end());
  EXPECT_EQ(it, list.begin() + 1);
}

TEST(VArray, FindNotExists)
{
  VW::v_array<int> list;
  list.push_back(1);
  list.push_back(2);
  list.push_back(6);
  list.push_back(4);

  auto it = std::find(list.begin(), list.end(), 11);
  EXPECT_EQ(it, list.end());
}

TEST(VArray, Back)
{
  VW::v_array<int> list;
  list.push_back(1);
  list.push_back(2);
  EXPECT_EQ(2, list.back());
}

TEST(VArray, EraseSingleElementSingleElementArray)
{
  VW::v_array<int> list;
  list.push_back(1);
  EXPECT_EQ(std::size_t(1), list.size());
  list.erase(list.begin());
  EXPECT_EQ(std::size_t(0), list.size());
}

TEST(VArray, EraseSingleElementReuseArray)
{
  VW::v_array<int> list;
  list.push_back(1);
  EXPECT_EQ(std::size_t(1), list.size());
  list.erase(list.begin());
  EXPECT_EQ(std::size_t(0), list.size());
  list.push_back(2);
  list.push_back(3);
  EXPECT_EQ(std::size_t(2), list.size());
}

TEST(VArray, EraseRange)
{
  VW::v_array<int> list;
  list.push_back(1);
  list.push_back(2);
  list.push_back(3);
  list.push_back(4);
  EXPECT_EQ(std::size_t(4), list.size());
  auto it = list.erase(list.begin() + 1, list.begin() + 3);
  EXPECT_EQ(*it, 4);
  EXPECT_EQ(list[0], 1);
  EXPECT_EQ(list[1], 4);
  EXPECT_EQ(std::size_t(2), list.size());
}

TEST(VArray, EraseRangeZeroWidth)
{
  VW::v_array<int> list;
  list.push_back(1);
  list.push_back(2);
  EXPECT_EQ(std::size_t(2), list.size());
  list.erase(list.begin() + 1, list.begin() + 1);
  EXPECT_EQ(std::size_t(2), list.size());
}

TEST(VArray, EraseLastElement)
{
  VW::v_array<int> list;
  list.push_back(5);
  list.push_back(3);
  EXPECT_EQ(std::size_t(2), list.size());
  auto it = list.erase(list.begin() + 1);
  EXPECT_EQ(std::size_t(1), list.size());
  EXPECT_EQ(list[0], 5);
  EXPECT_EQ(it, list.end());
}

TEST(VArray, InsertFromEmpty)
{
  VW::v_array<int> list;
  list.insert(list.begin(), 1);

  EXPECT_EQ(std::size_t(1), list.size());
  EXPECT_EQ(1, list[0]);
}

TEST(VArray, InsertCheckIterator)
{
  VW::v_array<int> list;
  auto it = list.insert(list.begin(), 47);

  EXPECT_EQ(std::size_t(1), list.size());
  EXPECT_EQ(47, list[0]);
  EXPECT_EQ(47, *it);
}

TEST(VArray, InsertEndIterator)
{
  VW::v_array<int> list;
  auto it = list.insert(list.end(), 22);

  EXPECT_EQ(std::size_t(1), list.size());
  EXPECT_EQ(22, list[0]);
  EXPECT_EQ(22, *it);
}

TEST(VArray, InsertMultipleInsert)
{
  VW::v_array<int> list;
  list.insert(list.begin(), 1);
  list.insert(list.end(), 2);

  EXPECT_EQ(std::size_t(2), list.size());
  EXPECT_EQ(1, list[0]);
  EXPECT_EQ(2, list[1]);

  list.insert(list.begin() + 1, 33);
  EXPECT_EQ(std::size_t(3), list.size());
  EXPECT_EQ(1, list[0]);
  EXPECT_EQ(33, list[1]);
  EXPECT_EQ(2, list[2]);

  list.insert(list.begin(), 8);
  EXPECT_EQ(std::size_t(4), list.size());
  EXPECT_EQ(8, list[0]);
  EXPECT_EQ(1, list[1]);
  EXPECT_EQ(33, list[2]);
  EXPECT_EQ(2, list[3]);

  list.insert(list.begin() + 2, 13);
  EXPECT_EQ(std::size_t(5), list.size());
  EXPECT_EQ(8, list[0]);
  EXPECT_EQ(1, list[1]);
  EXPECT_EQ(13, list[2]);
  EXPECT_EQ(33, list[3]);
  EXPECT_EQ(2, list[4]);

  list.insert(list.begin() + 1, 41);
  EXPECT_EQ(std::size_t(6), list.size());
  EXPECT_EQ(8, list[0]);
  EXPECT_EQ(41, list[1]);
  EXPECT_EQ(1, list[2]);
  EXPECT_EQ(13, list[3]);
  EXPECT_EQ(33, list[4]);
  EXPECT_EQ(2, list[5]);
}

TEST(VArray, InsertInLoop)
{
  VW::v_array<int> list;
  const auto num_values_to_insert = 1000;
  for (auto i = 0; i < num_values_to_insert; i++) { list.insert(list.begin(), i); }

  EXPECT_EQ(std::size_t(num_values_to_insert), list.size());

  for (auto i = 0; i < num_values_to_insert; i++) { EXPECT_EQ(num_values_to_insert - i - 1, list[i]); }
}

TEST(VArray, InsertRange)
{
  VW::v_array<int> list;
  std::vector<int> to_insert = {1, 2};
  list.insert(list.begin(), to_insert.begin(), to_insert.end());

  EXPECT_EQ(std::size_t(2), list.size());
  EXPECT_EQ(1, list[0]);
  EXPECT_EQ(2, list[1]);

  to_insert = {33, 44};
  list.insert(list.begin() + 1, to_insert.begin(), to_insert.end());
  EXPECT_EQ(std::size_t(4), list.size());
  EXPECT_EQ(1, list[0]);
  EXPECT_EQ(33, list[1]);
  EXPECT_EQ(44, list[2]);
  EXPECT_EQ(2, list[3]);

  to_insert = {22, 55};
  list.insert(list.end(), to_insert.begin(), to_insert.end());
  EXPECT_EQ(std::size_t(6), list.size());
  EXPECT_EQ(1, list[0]);
  EXPECT_EQ(33, list[1]);
  EXPECT_EQ(44, list[2]);
  EXPECT_EQ(2, list[3]);
  EXPECT_EQ(22, list[4]);
  EXPECT_EQ(55, list[5]);
}

TEST(VArray, InsertRangeEmptyEnd)
{
  VW::v_array<int> list;
  std::vector<int> to_insert = {1, 2};
  list.insert(list.end(), to_insert.begin(), to_insert.end());

  EXPECT_EQ(std::size_t(2), list.size());
  EXPECT_EQ(1, list[0]);
  EXPECT_EQ(2, list[1]);
}
