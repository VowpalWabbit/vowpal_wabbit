// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/io_buf.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing;

using align_t = VW::desired_align::align_t;
using flatbuffer_t = VW::desired_align::flatbuffer_t;
using VW::desired_align;

struct positioned_ptr
{
  size_t allocation;
  void* allocation_unit;
  int8_t* p;
  static_assert(sizeof(int8_t) == 1, "int8_t is not 1 byte");

  positioned_ptr(size_t allocation)
      : allocation(allocation), allocation_unit(malloc(allocation)), p(reinterpret_cast<int8_t*>(allocation_unit))
  {
  }
  positioned_ptr(const positioned_ptr&) = delete;
  positioned_ptr(positioned_ptr&& original) : allocation(original.allocation), allocation_unit(original.allocation_unit), p(original.p)
  {
    original.allocation_unit = nullptr;
    original.allocation = 0;
    original.p = nullptr;
  }

  ~positioned_ptr()
  {
    if (allocation_unit != nullptr) { free(allocation_unit); }
  }

  void realign(align_t alignment, align_t offset)
  {
    size_t base_address = reinterpret_cast<size_t>(allocation_unit);
    size_t base_offset = base_address % alignment;

    size_t padding = alignment - base_offset + offset;
    assert(padding < allocation);

    p += padding;
  }
};

template <typename T>
positioned_ptr prepare_pointer(align_t offset)
{
  align_t base_alignment = alignof(T);
  size_t playground = 2 * sizeof(T);

  positioned_ptr ptr(playground);
  ptr.realign(base_alignment, offset);

  return ptr;
}

template <>
positioned_ptr prepare_pointer<flatbuffer_t>(align_t offset)
{
  align_t base_alignment = flatbuffer_t::align;
  size_t playground = 16;

  positioned_ptr ptr(playground);
  ptr.realign(base_alignment, offset);

  return ptr;
}

template <typename T>
void test_desired_alignment_checker(align_t offset)
{
  desired_align da = desired_align::align_for<T>(offset);

  for (size_t i_offset = 0; i_offset < da.align; i_offset++)
  {
    positioned_ptr ptr = prepare_pointer<T>(i_offset);

    if (i_offset == offset) { EXPECT_TRUE(da.is_aligned(ptr.p)); }
    else { EXPECT_FALSE(da.is_aligned(ptr.p)); }
  }
}

template <typename T>
void test_all_alignments()
{
  for (align_t i_offset = 0; i_offset < alignof(T); i_offset++) { test_desired_alignment_checker<T>(i_offset); }
}

TEST(DesiredAlign, TestsAlignmentCorrectly)
{
  test_all_alignments<int8_t>();
  test_all_alignments<int16_t>();
  test_all_alignments<int32_t>();
  test_all_alignments<int64_t>();
  test_all_alignments<uint8_t>();
  test_all_alignments<uint16_t>();
  test_all_alignments<uint32_t>();
  test_all_alignments<float>();
  test_all_alignments<double>();
  test_all_alignments<char>();
  test_all_alignments<flatbuffer_t>();
}