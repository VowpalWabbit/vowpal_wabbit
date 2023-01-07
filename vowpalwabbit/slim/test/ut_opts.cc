#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "vw/slim/opts.h"
using namespace ::testing;
using namespace vw_slim;

TEST(CommandLineOptionsSlim, Parsing)
{
  EXPECT_THAT(find_opt("-q abc", "-q"), ElementsAre("abc"));
  EXPECT_THAT(find_opt("-q a -q b", "-q"), ElementsAre("a", "b"));
  EXPECT_THAT(find_opt("-q a -d   -q  b", "-q"), ElementsAre("a", "b"));
  EXPECT_THAT(find_opt("-q a -d -q  b -q -q abc", "-q"), ElementsAre("a", "b", "abc"));
}

TEST(CommandLineOptionsSlim, ParsingEmpty)
{
  EXPECT_THAT(find_opt("-a b -qd ", "-q").size(), 0);
  EXPECT_THAT(find_opt("", "-q").size(), 0);
  EXPECT_THAT(find_opt("-a", "-q").size(), 0);
  EXPECT_THAT(find_opt("-q", "-q").size(), 0);
  EXPECT_THAT(find_opt("-q ", "-q").size(), 0);
  EXPECT_THAT(find_opt("-q -d", "-q").size(), 0);
}

TEST(CommandLineOptionsSlim, ParsingFloat)
{
  float value;
  EXPECT_TRUE(find_opt_float("--epsilon 0.5", "--epsilon", value));
  ASSERT_FLOAT_EQ(0.5f, value);

  EXPECT_TRUE(find_opt_float("--lambda -2", "--lambda", value));
  ASSERT_FLOAT_EQ(-2.f, value);

  EXPECT_FALSE(find_opt_float("--epsilon", "--epsilon", value));
  EXPECT_FALSE(find_opt_float("--epsilon 0.5 --epsilon 0.4", "--epsilon", value));
}

TEST(CommandLineOptionsSlim, ParsingInt)
{
  int value;
  EXPECT_TRUE(find_opt_int("--bag 2", "--bag", value));
  ASSERT_EQ(2, value);

  EXPECT_FALSE(find_opt_int("--bag", "--bag", value));
  EXPECT_FALSE(find_opt_int("--bag 2 --bag 3", "--epsilon", value));
}
