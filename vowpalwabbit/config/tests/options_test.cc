// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/config/options.h"

#include "vw/config/option.h"
#include "vw/config/options_name_extractor.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

using namespace VW::config;

template <typename T>
std::shared_ptr<T> to_opt_ptr(option_builder<T>&& builder)
{
  return std::dynamic_pointer_cast<T>(option_builder<T>::finalize(std::move(builder)));
}

template <typename T>
std::shared_ptr<T> to_opt_ptr(option_builder<T>& builder)
{
  return to_opt_ptr(std::move(builder));
}

TEST(Options, MakeOptionAndCustomize)
{
  int loc = 0;
  auto opt = to_opt_ptr(make_option("opt", loc).default_value(4).help("Help text").keep().short_name("t"));

  EXPECT_EQ(opt->m_name, "opt");
  EXPECT_EQ(opt->default_value_supplied(), true);
  EXPECT_EQ(opt->default_value(), 4);
  EXPECT_EQ(opt->m_help, "Help text");
  EXPECT_EQ(opt->m_keep, true);
  EXPECT_EQ(opt->m_short_name, "t");
  EXPECT_EQ(opt->m_type_hash, typeid(decltype(loc)).hash_code());
  opt->value(5, true);
  EXPECT_EQ(loc, 5);
}

TEST(Options, TypedArgumentEquality)
{
  int int_loc;
  int int_loc_other;
  float float_loc;
  auto arg1 = to_opt_ptr(make_option("int_opt", int_loc).default_value(4).help("Help text").keep().short_name("t"));

  auto arg2 =
      to_opt_ptr(make_option("int_opt", int_loc_other).default_value(4).help("Help text").keep().short_name("t"));

  auto param_3 = to_opt_ptr(make_option("float_opt", float_loc).default_value(3.2f).short_name("f"));

  base_option* b1 = static_cast<base_option*>(arg1.get());
  base_option* b2 = static_cast<base_option*>(arg2.get());
  base_option* b3 = static_cast<base_option*>(param_3.get());

  EXPECT_TRUE(*arg1 == *arg2);
  EXPECT_TRUE(*b1 == *b2);
  EXPECT_TRUE(*b1 != *b3);
}

TEST(Options, CreateArgumentGroup)
{
  std::string loc;
  std::vector<std::string> loc2;
  option_group_definition ag("g1");
  ag(make_option("opt1", loc).keep());
  ag(make_option("opt2", loc));
  ag.add(make_option("opt3", loc2));
  ag.add(make_option("opt4", loc2).keep());

  EXPECT_EQ(ag.m_name, "g1 Options");
  EXPECT_EQ(ag.m_options[0]->m_name, "opt1");
  EXPECT_EQ(ag.m_options[0]->m_keep, true);
  EXPECT_EQ(ag.m_options[0]->m_type_hash, typeid(decltype(loc)).hash_code());

  EXPECT_EQ(ag.m_options[1]->m_name, "opt2");
  EXPECT_EQ(ag.m_options[1]->m_type_hash, typeid(decltype(loc)).hash_code());

  EXPECT_EQ(ag.m_options[2]->m_name, "opt3");
  EXPECT_EQ(ag.m_options[2]->m_type_hash, typeid(decltype(loc2)).hash_code());

  EXPECT_EQ(ag.m_options[3]->m_name, "opt4");
  EXPECT_EQ(ag.m_options[3]->m_keep, true);
  EXPECT_EQ(ag.m_options[3]->m_type_hash, typeid(decltype(loc2)).hash_code());
}

TEST(Options, NameExtractionFromOptionGroup)
{
  std::string loc;
  std::vector<std::string> loc2;
  option_group_definition ag("g1");
  ag(make_option("im_necessary", loc).keep().necessary());
  ag(make_option("opt2", loc));
  ag.add(make_option("opt3", loc2));
  ag.add(make_option("opt4", loc2).keep());

  auto name_extractor = options_name_extractor();
  // result should always be false
  bool result = name_extractor.add_parse_and_check_necessary(ag);

  EXPECT_EQ(name_extractor.generated_name, "im_necessary");
  EXPECT_EQ(result, false);
  // was_supplied will always return false
  EXPECT_EQ(name_extractor.was_supplied("opt2"), false);
  EXPECT_EQ(name_extractor.was_supplied("random"), false);

  // should throw since we validate that reductions should use add_parse_and_check_necessary
  EXPECT_THROW(name_extractor.add_and_parse(ag), VW::vw_exception);
}

TEST(Options, NameExtractionMultiNecessary)
{
  std::string loc;
  std::vector<std::string> loc2;
  option_group_definition ag("g1");
  ag(make_option("im_necessary", loc).keep().necessary());
  ag(make_option("opt2", loc).necessary());
  ag.add(make_option("opt3", loc2));
  ag.add(make_option("opt4", loc2).keep());

  auto name_extractor = options_name_extractor();
  // result should always be false
  bool result = name_extractor.add_parse_and_check_necessary(ag);

  EXPECT_EQ(name_extractor.generated_name, "im_necessary_opt2");
  EXPECT_EQ(result, false);
  // was_supplied will always return false
  EXPECT_EQ(name_extractor.was_supplied("opt2"), false);
  EXPECT_EQ(name_extractor.was_supplied("random"), false);

  // should throw since we validate that reductions should use add_parse_and_check_necessary
  EXPECT_THROW(name_extractor.add_and_parse(ag), VW::vw_exception);
}

TEST(Options, NameExtractionShouldThrow)
{
  std::string loc;
  std::vector<std::string> loc2;
  option_group_definition ag("g1");
  ag(make_option("im_necessary", loc).keep());
  ag(make_option("opt2", loc));
  ag.add(make_option("opt3", loc2));
  ag.add(make_option("opt4", loc2).keep());

  auto name_extractor = options_name_extractor();

  // should throw since no .necessary() is defined
  EXPECT_THROW(name_extractor.add_parse_and_check_necessary(ag), VW::vw_exception);

  // should throw since these methods will never be implemented by options_name_extractor
  std::vector<std::string> warnings;
  EXPECT_THROW(warnings = name_extractor.check_unregistered(), VW::vw_exception);
  EXPECT_THROW(name_extractor.insert("opt2", "blah"), VW::vw_exception);
  EXPECT_THROW(name_extractor.replace("opt2", "blah"), VW::vw_exception);
}

TEST(Options, NameExtractionRecycle)
{
  std::string loc;
  std::vector<std::string> loc2;
  option_group_definition ag("g1");
  ag(make_option("im_necessary", loc).keep().necessary());
  ag(make_option("opt2", loc).necessary());
  ag.add(make_option("opt3", loc2));
  ag.add(make_option("opt4", loc2).keep());

  auto name_extractor = options_name_extractor();
  // result should always be false
  bool result = name_extractor.add_parse_and_check_necessary(ag);

  EXPECT_EQ(name_extractor.generated_name, "im_necessary_opt2");
  EXPECT_EQ(result, false);

  option_group_definition ag2("g2");
  ag2(make_option("im_necessary_v2", loc).keep().necessary());
  ag2(make_option("opt2", loc).necessary());
  ag2.add(make_option("opt3", loc2));
  ag2.add(make_option("opt4", loc2).keep());

  // result should always be false
  result = name_extractor.add_parse_and_check_necessary(ag2);

  EXPECT_EQ(name_extractor.generated_name, "im_necessary_v2_opt2");
  EXPECT_EQ(result, false);
}

TEST(Options, SetTags)
{
  typed_option<bool> opt("my_opt");
  opt.set_tags(std::vector<std::string>{"tagb", "taga", "tagc"});
  ASSERT_THAT(opt.get_tags(), ::testing::ElementsAre("taga", "tagb", "tagc"));
}

TEST(Options, SetTagsDuplicate)
{
  typed_option<bool> opt("my_opt");
  EXPECT_THROW(opt.set_tags(std::vector<std::string>{"taga", "tagb", "tagb"}), VW::vw_exception);
}

TEST(Options, SetTagsInvalidName)
{
  typed_option<bool> opt("my_opt");
  EXPECT_THROW(opt.set_tags(std::vector<std::string>{"tag1"}), VW::vw_exception);
  EXPECT_THROW(opt.set_tags(std::vector<std::string>{"Tag"}), VW::vw_exception);
  EXPECT_THROW(opt.set_tags(std::vector<std::string>{"a b"}), VW::vw_exception);
  EXPECT_THROW(opt.set_tags(std::vector<std::string>{"t-a-g"}), VW::vw_exception);
}
