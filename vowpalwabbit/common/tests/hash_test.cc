// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/common/hash.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(UniformHash, HashZeroSeed)
{
  EXPECT_EQ(VW::uniform_hash("t", 1, 0), 3397902157);
  EXPECT_EQ(VW::uniform_hash("te", 2, 0), 3988319771);
  EXPECT_EQ(VW::uniform_hash("tes", 3, 0), 196677210);
  EXPECT_EQ(VW::uniform_hash("test", 4, 0), 3127628307);
  EXPECT_EQ(VW::uniform_hash("tested", 6, 0), 2247989476);
  EXPECT_EQ(VW::uniform_hash("8hv20cjwicnsj vw m000'.'.][][]...!!@3", 37, 0), 4212741639);
}

TEST(UniformHash, HashNonZeroSeed)
{
  EXPECT_EQ(VW::uniform_hash("t", 1, 25436347), 960607349);
  EXPECT_EQ(VW::uniform_hash("te", 2, 25436347), 2834341637);
  EXPECT_EQ(VW::uniform_hash("tes", 3, 25436347), 1163171263);
  EXPECT_EQ(VW::uniform_hash("test", 4, 25436347), 1661171760);
  EXPECT_EQ(VW::uniform_hash("tested", 6, 25436347), 3592599130);
  EXPECT_EQ(VW::uniform_hash("8hv20cjwicnsj vw m000'.'.][][]...!!@3", 37, 25436347), 2503360452);
}

TEST(UniformHash, NonAsciiUTF8)
{
  std::string str = "aஜெய்";
  EXPECT_EQ(VW::uniform_hash(str.c_str(), str.size(), 0), 2574050673);
  EXPECT_EQ(VW::uniform_hash(str.c_str(), str.size(), 2493003127), 2630240042);

  str = "𝓜ƝȎ𝚸𝑄";
  EXPECT_EQ(VW::uniform_hash(str.c_str(), str.size(), 0), 1338280499);
  EXPECT_EQ(VW::uniform_hash(str.c_str(), str.size(), 2493003127), 3575816965);

  str = "ḿ𝕟𝐨𝝔𝕢ṛ𝓼тú𝔳ẃ";
  EXPECT_EQ(VW::uniform_hash(str.c_str(), str.size(), 0), 2662276298);
  EXPECT_EQ(VW::uniform_hash(str.c_str(), str.size(), 2493003127), 4044487226);
}

TEST(UniformHash, BytesInSignedNegativeRange)
{
  EXPECT_EQ(VW::uniform_hash("\xd6", 1, 0), 2340286700);
  EXPECT_EQ(VW::uniform_hash("\xd6\xd3", 2, 0), 1807922709);
  EXPECT_EQ(VW::uniform_hash("\xd6\xd3\xc3", 3, 0), 253517139);
  EXPECT_EQ(VW::uniform_hash("\xd6\xd3\xc3\xe3", 4, 0), 3334312508);
  EXPECT_EQ(VW::uniform_hash("\xd6\xd3\xc3\xe3\xa3", 5, 0), 1293710453);

  EXPECT_EQ(VW::uniform_hash("\xd6", 1, 1342134), 1291967587);
  EXPECT_EQ(VW::uniform_hash("\xd6\xd3", 2, 1342134), 2610972564);
  EXPECT_EQ(VW::uniform_hash("\xd6\xd3\xc3", 3, 1342134), 2367428039);
  EXPECT_EQ(VW::uniform_hash("\xd6\xd3\xc3\xe3", 4, 1342134), 1455891233);
  EXPECT_EQ(VW::uniform_hash("\xd6\xd3\xc3\xe3\xa3", 5, 1342134), 1029777931);
}
