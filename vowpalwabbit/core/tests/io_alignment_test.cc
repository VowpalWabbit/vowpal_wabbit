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

// positioned_ptr is a helper class to allocate a pointer at a partiular alignment+offset, for use in
// testing whether desired_align functions properly when testing the alignment of pointers.
struct positioned_ptr
{
  size_t allocation;      // The size of the allocated buffer - used to ensure we don't run off the end of the buffer.
  void* allocation_unit;  // The actual allocated buffer within which we will be positioning our test pointer.
  int8_t* p;              // The alignable pointer. Once realign() is called, it will be the pointer we are testing.
  static_assert(sizeof(int8_t) == 1, "int8_t is not 1 byte");  // this should only happen IFF someone messed with
                                                               // typedefs but it would invalidate the test.

  positioned_ptr(size_t allocation)
      : allocation(allocation), allocation_unit(malloc(allocation)), p(reinterpret_cast<int8_t*>(allocation_unit))
  {
  }
  positioned_ptr(const positioned_ptr&) = delete;
  positioned_ptr(positioned_ptr&& original)
      : allocation(original.allocation), allocation_unit(original.allocation_unit), p(original.p)
  {
    original.allocation_unit = nullptr;
    original.allocation = 0;
    original.p = nullptr;
  }

  ~positioned_ptr()
  {
    if (allocation_unit != nullptr) { free(allocation_unit); }
  }

  // Move p so that it reflects the desired alignment. Technically this can run past the
  // end of the allocation unit, which is largely fine because we are never expecting to
  // read this pointer anyways, and are only using it for math.
  //
  // At the same time, if this was ever promoted to live code, rather than test code, we
  // would want to fix that edge-case beyond being an assert-check.
  void realign(align_t alignment, align_t offset)
  {
    size_t base_address = reinterpret_cast<size_t>(allocation_unit);
    size_t base_offset = base_address % alignment;

    size_t padding = alignment - base_offset + offset;
    assert(padding < allocation);

    p = reinterpret_cast<int8_t*>(allocation_unit) + padding;
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

TEST(IoAlignedReads, BasicAlignedReadTest)
{
  std::vector<uint16_t> data = {0x1050, 0x2060, 0x3070, 0x4080, 0x0000, 0x0000};

  VW::io_buf buf;
  buf.add_file(VW::io::create_buffer_view((const char*)&data[0], data.size() * sizeof(uint16_t)));

  desired_align uint16_align = desired_align::align_for<uint16_t>();
  desired_align uint64_align = desired_align::align_for<uint64_t>();

  char* p = nullptr;
  buf.buf_read(p, 2 * sizeof(uint16_t), uint16_align);
  EXPECT_TRUE(uint16_align.is_aligned(p));
  char* first_p = p;

  buf.buf_read(p, 4 * sizeof(uint16_t), uint64_align);
  EXPECT_TRUE(uint64_align.is_aligned(p));
  char* second_p = p;

  EXPECT_EQ(first_p, second_p);  // make sure that we triggered the move-back code
}