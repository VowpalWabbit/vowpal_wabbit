// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/common/vw_exception.h"
#include "vw/core/numeric_casts.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(NumericCastTests, NumericCast)
{
  // Correct negative
  EXPECT_EQ(VW::cast_to_smaller_type<int8_t>(static_cast<int32_t>(-7)), -7);
  // Correct positive
  EXPECT_EQ(VW::cast_to_smaller_type<int8_t>(static_cast<int32_t>(4)), 4);
  // Too small
  EXPECT_THROW(VW::cast_to_smaller_type<int8_t>(static_cast<int32_t>(-500)), VW::vw_exception);
  // Too large
  EXPECT_THROW(VW::cast_to_smaller_type<int8_t>(static_cast<int32_t>(50000)), VW::vw_exception);

  // Negative
  EXPECT_THROW(VW::cast_signed_to_unsigned<uint32_t>(static_cast<int32_t>(-5)), VW::vw_exception);
  // Correct
  EXPECT_EQ(VW::cast_signed_to_unsigned<uint32_t>(static_cast<int32_t>(10)), 10);
  // Larger unsigned to smaller signed
  EXPECT_EQ(VW::cast_signed_to_unsigned<uint8_t>(static_cast<int32_t>(10)), 10);
}

TEST(NumericCastTests, CastUnsignedToSigned)
{
  EXPECT_THROW(VW::cast_unsigned_to_signed<int8_t>(static_cast<uint32_t>(1000)), VW::vw_exception);
  EXPECT_EQ(VW::cast_unsigned_to_signed<int8_t>(static_cast<uint32_t>(60)), 60);
  EXPECT_EQ(VW::cast_unsigned_to_signed<int8_t>(static_cast<uint8_t>(60)), 60);
  EXPECT_THROW(VW::cast_unsigned_to_signed<int8_t>(static_cast<uint8_t>(200)), VW::vw_exception);
}