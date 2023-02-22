// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/vw.h"
#include "vw/csv_parser/parse_example_csv.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(CsvParser, ComplexCsvSimpleLabelExamples)
{
  /*
   * Equivalent VW Text format:
   * 1 2 'te"st,tst,"|sepal1 length:5.1 |sepal:-2.2 width:3.5 |petal:2 length":1.4 width:0.2 | type:1
   * 2 'te""st|sepal:-2.2 width:4.9 |petal:2 length":3 width:-1.4 | type:110 k:1 :-2
   */
  std::string example_string =
      // Header
      "\xef\xbb\xbf\"sepal1|length\",sepal|width,\"petal|length\"\"\",petal|width,"
      "_label,type,_tag,\"k\",,\xef\xbb\xbf\n"
      // Example 1
      "\f5.1,3.5,1.4,.2,1 2,1,\"'te\"\"st,tst,\"\"\",0,,\v\n"
      // Example 2
      "\f0,4.9,3.0,-1.4,\"2\",0x6E,'te\"\"st,1.0,-2,\v";

  auto vw = VW::initialize(
      vwtest::make_args("--no_stdin", "--quiet", "-a", "--csv", "--csv_ns_value", "sepal:-2.2,petal:2,sepal1:1"));
  VW::io_buf buffer;
  buffer.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
  VW::multi_ex examples;

  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_EQ(vw->example_parser->reader(vw.get(), buffer, examples), 1);

  // Check example 1 label and tag
  EXPECT_FLOAT_EQ(examples[0]->l.simple.label, 1.f);
  const auto& red_features_exp1 =
      examples[0]->ex_reduction_features.template get<VW::simple_label_reduction_features>();
  EXPECT_FLOAT_EQ(red_features_exp1.weight, 2.f);
  EXPECT_EQ(examples[0]->tag.size(), 11);
  std::string example1_tag = {examples[0]->tag.data(), examples[0]->tag.size()};
  // ' at the start should be removed
  // CSV separators inside the double quotes sre all escaped
  // The double quotes for escape should be removed
  // Auto remove the outer quotes
  EXPECT_EQ(example1_tag, "te\"st,tst,\"");

  // Check example 1 feature numbers
  EXPECT_EQ(examples[0]->feature_space['s'].size(), 2);
  EXPECT_EQ(examples[0]->feature_space['p'].size(), 2);
  // Zero and empty values should be ignored
  EXPECT_EQ(examples[0]->feature_space[' '].size(), 1);
  EXPECT_EQ(examples[0]->feature_space['\''].size(), 0);
  EXPECT_EQ(examples[0]->feature_space['"'].size(), 0);
  EXPECT_EQ(examples[0]->feature_space['_'].size(), 0);

  // Check example 1 namespace numbers
  EXPECT_EQ(examples[0]->feature_space['s'].namespace_extents.size(), 2);
  EXPECT_EQ(examples[0]->feature_space['p'].namespace_extents.size(), 1);
  EXPECT_EQ(examples[0]->feature_space[' '].namespace_extents.size(), 1);

  // Check example 1 feature value
  // \f\v should be trimmed
  EXPECT_TRUE((std::abs(examples[0]->feature_space['s'].values[0] - 5.1) < 0.01 &&
                  std::abs(examples[0]->feature_space['s'].values[1] + 7.7) < 0.01) ||
      (std::abs(examples[0]->feature_space['s'].values[0] + 7.7) < 0.01 &&
          std::abs(examples[0]->feature_space['s'].values[1] - 5.1) < 0.01));
  EXPECT_FLOAT_EQ(examples[0]->feature_space['p'].values[0], 2.8);
  EXPECT_FLOAT_EQ(examples[0]->feature_space['p'].values[1], 0.4);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[0], 1);

  // Check example 1 namespace names and feature names
  // \xef\xbb\xbf\ should be trimmed
  EXPECT_TRUE((examples[0]->feature_space['s'].space_names[0].ns == "sepal1" &&
                  examples[0]->feature_space['s'].space_names[0].name == "length" &&
                  examples[0]->feature_space['s'].space_names[1].ns == "sepal" &&
                  examples[0]->feature_space['s'].space_names[1].name == "width") ||
      (examples[0]->feature_space['s'].space_names[0].ns == "sepal" &&
          examples[0]->feature_space['s'].space_names[0].name == "width" &&
          examples[0]->feature_space['s'].space_names[1].ns == "sepal1" &&
          examples[0]->feature_space['s'].space_names[1].name == "length"));
  EXPECT_EQ(examples[0]->feature_space['p'].space_names[0].ns, "petal");
  // The double quotes for escape should be removed
  EXPECT_EQ(examples[0]->feature_space['p'].space_names[0].name, "length\"");
  EXPECT_EQ(examples[0]->feature_space['p'].space_names[1].ns, "petal");
  EXPECT_EQ(examples[0]->feature_space['p'].space_names[1].name, "width");
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[0].ns, " ");
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[0].name, "type");

  VW::finish_example(*vw, *examples[0]);
  examples.clear();

  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_EQ(vw->example_parser->reader(vw.get(), buffer, examples), 1);

  // Check example 2 label and tag
  EXPECT_FLOAT_EQ(examples[0]->l.simple.label, 2.f);
  const auto& red_features_exp2 =
      examples[0]->ex_reduction_features.template get<VW::simple_label_reduction_features>();
  EXPECT_FLOAT_EQ(red_features_exp2.weight, 1.f);
  EXPECT_EQ(examples[0]->tag.size(), 6);
  std::string example2_tag = {examples[0]->tag.data(), examples[0]->tag.size()};
  // The double quotes appear outside the quotes (not for escape) should not be removed
  EXPECT_EQ(example2_tag, "te\"\"st");

  // Check example 2 feature numbers
  EXPECT_EQ(examples[0]->feature_space['s'].size(), 1);
  EXPECT_EQ(examples[0]->feature_space['p'].size(), 2);
  EXPECT_EQ(examples[0]->feature_space[' '].size(), 3);
  EXPECT_EQ(examples[0]->feature_space['\''].size(), 0);
  EXPECT_EQ(examples[0]->feature_space['"'].size(), 0);
  EXPECT_EQ(examples[0]->feature_space['_'].size(), 0);

  // Check example 2 namespace numbers
  EXPECT_EQ(examples[0]->feature_space['s'].namespace_extents.size(), 1);
  EXPECT_EQ(examples[0]->feature_space['p'].namespace_extents.size(), 1);
  EXPECT_EQ(examples[0]->feature_space[' '].namespace_extents.size(), 1);

  // Check example 2 feature value
  EXPECT_FLOAT_EQ(examples[0]->feature_space['s'].values[0], -10.78);
  EXPECT_FLOAT_EQ(examples[0]->feature_space['p'].values[0], 6);
  EXPECT_FLOAT_EQ(examples[0]->feature_space['p'].values[1], -2.8);
  // Should recognize 0x6E
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[0], 110);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[1], 1);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[2], -2);

  // Check example 2 namespace names and feature names
  EXPECT_EQ(examples[0]->feature_space['s'].space_names[0].ns, "sepal");
  EXPECT_EQ(examples[0]->feature_space['s'].space_names[0].name, "width");
  EXPECT_EQ(examples[0]->feature_space['p'].space_names[0].ns, "petal");
  EXPECT_EQ(examples[0]->feature_space['p'].space_names[0].name, "length\"");
  EXPECT_EQ(examples[0]->feature_space['p'].space_names[1].ns, "petal");
  EXPECT_EQ(examples[0]->feature_space['p'].space_names[1].name, "width");
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[0].ns, " ");
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[0].name, "type");
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[1].ns, " ");
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[1].name, "k");
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[2].ns, " ");
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[2].name, "");

  VW::finish_example(*vw, *examples[0]);
}

TEST(CsvParser, MultipleFileExamples)
{
  /*
   * Equivalent VW Text format:
   * 4 a| a:1 b:2 c:3
   */
  auto vw = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--csv", "--csv_separator", "\\t"));

  VW::io_buf buffer;
  VW::multi_ex examples;

  // Read the first file
  std::string file1_string =
      // Header
      "a\tb\tc\t_label\t_tag\n"
      // Example
      "1\t2\t3\t4\ta\n";
  buffer.add_file(VW::io::create_buffer_view(file1_string.data(), file1_string.size()));

  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_EQ(vw->example_parser->reader(vw.get(), buffer, examples), 1);

  EXPECT_FLOAT_EQ(examples[0]->l.simple.label, 4);
  EXPECT_EQ(examples[0]->tag.size(), 1);
  EXPECT_EQ(examples[0]->tag[0], 'a');
  EXPECT_EQ(examples[0]->feature_space[' '].size(), 3);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[0], 1);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[1], 2);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[2], 3);
  // Empty as not in audit mode
  EXPECT_EQ(examples[0]->feature_space[' '].space_names.size(), 0);

  VW::finish_example(*vw, *examples[0]);

  EXPECT_EQ(vw->example_parser->reader(vw.get(), buffer, examples), 0);
  examples.clear();

  // Read the second file
  /*
   * Equivalent VW Text format:
   * 6 bc| d:5
   */
  std::string file2_string =
      // Header
      "_tag\td\t_label\n"
      // Example
      "bc\t5\t6\n";
  buffer.add_file(VW::io::create_buffer_view(file2_string.data(), file2_string.size()));

  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_EQ(vw->example_parser->reader(vw.get(), buffer, examples), 1);

  EXPECT_FLOAT_EQ(examples[0]->l.simple.label, 6);
  EXPECT_EQ(examples[0]->tag.size(), 2);
  EXPECT_EQ(examples[0]->tag[0], 'b');
  EXPECT_EQ(examples[0]->tag[1], 'c');
  EXPECT_EQ(examples[0]->feature_space[' '].size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[0], 5);
  VW::finish_example(*vw, *examples[0]);
}

TEST(CsvParser, MulticlassExamples)
{
  /*
   * Equivalent VW Text format:
   * 1test | a:1 b:here c:3
   * 2test | a:2 b:3.0is c:NaN
   */
  std::string example_string =
      // Header
      "a;b;_label;c\n"
      // Example 1
      "1;here;\"1test\";3.0\n"
      // Example 2
      "2;3.0is;2test;NaN\n";

  auto vw = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "-a", "--csv", "--csv_separator", ";",
      "--chain_hash", "--named_labels", "2test,1test", "--oaa", "2"));

  VW::io_buf buffer;
  buffer.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
  VW::multi_ex examples;

  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_EQ(vw->example_parser->reader(vw.get(), buffer, examples), 1);

  // Check example 1 label
  EXPECT_EQ(examples[0]->l.multi.label, 2);

  // Check example 1 feature numbers
  EXPECT_EQ(examples[0]->feature_space[' '].size(), 3);

  // Check example 1 namespace numbers
  EXPECT_EQ(examples[0]->feature_space[' '].namespace_extents.size(), 1);

  // Check example 1 feature value
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[0], 1);
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[1].str_value, "here");
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[2], 3);

  // Check example 1 namespace names and feature names
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[0].ns, " ");
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[0].name, "a");
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[1].ns, " ");
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[1].name, "b");
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[2].ns, " ");
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[2].name, "c");

  VW::finish_example(*vw, *examples[0]);
  examples.clear();

  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_EQ(vw->example_parser->reader(vw.get(), buffer, examples), 1);

  // Check example 1 label
  EXPECT_EQ(examples[0]->l.multi.label, 1);

  // Check example 2 feature numbers
  EXPECT_EQ(examples[0]->feature_space[' '].size(), 3);

  // Check example 2 namespace numbers
  EXPECT_EQ(examples[0]->feature_space[' '].namespace_extents.size(), 1);

  // Check example 2 feature value
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[0], 2);
  // Test float parsing
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[1].str_value, "3.0is");
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[2].str_value, "NaN");

  // Check example 2 namespace names and feature names
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[0].ns, " ");
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[0].name, "a");
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[1].ns, " ");
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[1].name, "b");
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[2].ns, " ");
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[2].name, "c");

  VW::finish_example(*vw, *examples[0]);
}

TEST(CsvParser, ReplaceHeader)
{
  /*
   * Equivalent VW Text format:
   * 3 | :1 :2 :4
   */
  std::string example_string =
      // Header
      "a$b$_label$c\n"
      // Example 1
      "1$2$3$4\n";

  auto vw = VW::initialize(
      vwtest::make_args("--no_stdin", "--quiet", "--csv", "--csv_separator", "$", "--csv_header", ",,_label,"));

  VW::io_buf buffer;
  buffer.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
  VW::multi_ex examples;

  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_EQ(vw->example_parser->reader(vw.get(), buffer, examples), 1);

  // Check example 1 label
  EXPECT_FLOAT_EQ(examples[0]->l.simple.label, 3.f);
  const auto& red_features_exp1 =
      examples[0]->ex_reduction_features.template get<VW::simple_label_reduction_features>();
  EXPECT_FLOAT_EQ(red_features_exp1.weight, 1.f);

  // Check example 1 feature numbers
  EXPECT_EQ(examples[0]->feature_space[' '].size(), 3);
  EXPECT_EQ(examples[0]->feature_space['$'].size(), 0);

  // Check example 1 namespace numbers
  EXPECT_EQ(examples[0]->feature_space[' '].namespace_extents.size(), 1);

  // Check example 1 feature value
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[0], 1);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[1], 2);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[2], 4);

  // Check example 1 feature hash
  EXPECT_EQ(examples[0]->feature_space[' '].indices[0], 0);
  EXPECT_EQ(examples[0]->feature_space[' '].indices[1], 1);
  EXPECT_EQ(examples[0]->feature_space[' '].indices[2], 2);

  VW::finish_example(*vw, *examples[0]);
}

TEST(CsvParser, NoHeader)
{
  /*
   * Equivalent VW Text format:
   * 3 |n1 a:1 b:3 | a:4
   */
  std::string example_string =
      // Example 1
      "1&2&3&4\n";
  auto vw = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "-a", "--csv", "--csv_separator", "&",
      "--csv_no_file_header", "--csv_header", "n1|a,_label,n1|b,a"));

  VW::io_buf buffer;
  buffer.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
  VW::multi_ex examples;

  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_EQ(vw->example_parser->reader(vw.get(), buffer, examples), 1);

  // Check example 1 label
  EXPECT_FLOAT_EQ(examples[0]->l.simple.label, 2.f);
  const auto& red_features_exp1 =
      examples[0]->ex_reduction_features.template get<VW::simple_label_reduction_features>();
  EXPECT_FLOAT_EQ(red_features_exp1.weight, 1.f);

  // Check example 1 feature numbers
  EXPECT_EQ(examples[0]->feature_space['n'].size(), 2);
  EXPECT_EQ(examples[0]->feature_space[' '].size(), 1);
  EXPECT_EQ(examples[0]->feature_space['&'].size(), 0);

  // Check example 1 namespace numbers
  EXPECT_EQ(examples[0]->feature_space['n'].namespace_extents.size(), 1);

  // Check example 1 feature value
  EXPECT_FLOAT_EQ(examples[0]->feature_space['n'].values[0], 1);
  EXPECT_FLOAT_EQ(examples[0]->feature_space['n'].values[1], 3);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[0], 4);

  // Check example 1 namespace names and feature names
  EXPECT_EQ(examples[0]->feature_space['n'].space_names[0].ns, "n1");
  EXPECT_EQ(examples[0]->feature_space['n'].space_names[0].name, "a");
  EXPECT_EQ(examples[0]->feature_space['n'].space_names[1].ns, "n1");
  EXPECT_EQ(examples[0]->feature_space['n'].space_names[1].name, "b");
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[0].ns, " ");
  EXPECT_EQ(examples[0]->feature_space[' '].space_names[0].name, "a");
  // Hash check, different due to different namespaces
  EXPECT_TRUE(examples[0]->feature_space['n'].indices[0] != examples[0]->feature_space[' '].values[0]);

  VW::finish_example(*vw, *examples[0]);
}

TEST(CsvParser, EmptyHeaderAndExampleLine)
{
  std::string example_string =
      // Header
      ",,,\n"
      // New line
      ",,,\n";

  auto vw = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--csv"));

  VW::io_buf buffer;
  buffer.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
  VW::multi_ex examples;

  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_EQ(vw->example_parser->reader(vw.get(), buffer, examples), 1);
  EXPECT_EQ(examples[0]->is_newline, true);

  VW::finish_example(*vw, *examples[0]);
}

TEST(CsvParser, EmptyLineErrorThrown)
{
  std::string example_string =
      // Header
      "a,b,_label\n"
      // Empty line
      "\n";

  auto vw = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--csv"));

  VW::io_buf buffer;
  buffer.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
  VW::multi_ex examples;

  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_THROW(vw->example_parser->reader(vw.get(), buffer, examples), VW::vw_exception);

  VW::finish_example(*vw, *examples[0]);
}

TEST(CsvParser, ForbiddenCsvSeparatorErrorThrown)
{
  std::vector<std::string> csv_separator_forbid_chars = {"\"", "|", ":"};
  for (const std::string& csv_separator_forbid_char : csv_separator_forbid_chars)
  {
    EXPECT_THROW(VW::initialize(
                     vwtest::make_args("--no_stdin", "--quiet", "--csv", "--csv_separator", csv_separator_forbid_char)),
        VW::vw_exception);
  }
}

TEST(CsvParser, MulticharacterCsvSeparatorErrorThrown)
{
  EXPECT_THROW(
      VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--csv", "--csv_separator", "\\a")), VW::vw_exception);
}

TEST(CsvParser, NoHeaderWithoutSpecifyingErrorThrown)
{
  EXPECT_THROW(
      VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--csv", "--csv_no_file_header")), VW::vw_exception);
}

TEST(CsvParser, MalformedNamespaceValuePairNoElementErrorThrown)
{
  std::string example_string = " \n";
  auto vw = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--csv", "--csv_ns_value", ":5,"));
  VW::io_buf buffer;
  buffer.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
  VW::multi_ex examples;
  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_THROW(vw->example_parser->reader(vw.get(), buffer, examples), VW::vw_exception);

  VW::finish_example(*vw, *examples[0]);
}

TEST(CsvParser, MalformedNamespaceValuePairOneElementErrorThrown)
{
  std::string example_string = " \n";
  auto vw = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--csv", "--csv_ns_value", "a:5,0"));
  VW::io_buf buffer;
  buffer.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
  VW::multi_ex examples;
  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_THROW(vw->example_parser->reader(vw.get(), buffer, examples), VW::vw_exception);

  VW::finish_example(*vw, *examples[0]);
}

TEST(CsvParser, MalformedNamespaceValuePairThreeElementErrorThrown)
{
  std::string example_string = " \n";
  auto vw = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--csv", "--csv_ns_value", "c:5,b:a:6"));
  VW::io_buf buffer;
  buffer.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
  VW::multi_ex examples;
  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_THROW(vw->example_parser->reader(vw.get(), buffer, examples), VW::vw_exception);

  VW::finish_example(*vw, *examples[0]);
}

TEST(CsvParser, NanNamespaceValueErrorThrown)
{
  std::string example_string = " \n";
  auto vw = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--csv", "--csv_ns_value", "c:a"));
  VW::io_buf buffer;
  buffer.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
  VW::multi_ex examples;
  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_THROW(vw->example_parser->reader(vw.get(), buffer, examples), VW::vw_exception);

  VW::finish_example(*vw, *examples[0]);
}

TEST(CsvParser, MalformedHeaderErrorThrown)
{
  std::string example_string =
      // Malformed Header
      "a,b|c|d,_label\n"
      "1,2,3\n";

  auto vw = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--csv"));

  VW::io_buf buffer;
  buffer.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
  VW::multi_ex examples;

  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_THROW(vw->example_parser->reader(vw.get(), buffer, examples), VW::vw_exception);

  VW::finish_example(*vw, *examples[0]);
}

TEST(CsvParser, UnmatchingElementErrorThrown)
{
  std::string example_string =
      // Header has 3 elements
      "a,b,_label\n"
      // Example 1 has 2 elements
      "1,2\n"
      // Example 2 has 4 elements
      "3,4,5,6\n";

  auto vw = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--csv"));

  VW::io_buf buffer;
  buffer.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
  VW::multi_ex examples;

  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_THROW(vw->example_parser->reader(vw.get(), buffer, examples), VW::vw_exception);
  EXPECT_THROW(vw->example_parser->reader(vw.get(), buffer, examples), VW::vw_exception);

  VW::finish_example(*vw, *examples[0]);
}

TEST(CsvParser, UnmatchingQuotesErrorThrown)
{
  std::string example_string =
      // Malformed Header
      "abc,\"bd\"e,_label\n";

  auto vw = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--csv"));

  VW::io_buf buffer;
  buffer.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
  VW::multi_ex examples;

  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_THROW(vw->example_parser->reader(vw.get(), buffer, examples), VW::vw_exception);

  VW::finish_example(*vw, *examples[0]);
}

TEST(CsvParser, QuotesEolErrorThrown)
{
  std::string example_string =
      // Malformed Header
      "abc,\"bd\"\"e,_label\n";
  auto vw = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--csv"));

  VW::io_buf buffer;
  buffer.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
  VW::multi_ex examples;

  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_THROW(vw->example_parser->reader(vw.get(), buffer, examples), VW::vw_exception);

  VW::finish_example(*vw, *examples[0]);
}

/*
 * NOTE: Not explicitly support multiline examples,
 * as evidenced by the large number of empty fields.
 */
TEST(CsvParser, MultilineExamples)
{
  /*
   * Equivalent VW Text format:
   * shared | a:1 b:0.5
   * 0:0.1:0.75 | a:0.5 b:1 c:2
   * | a:1 c:3
   *
   * shared | s_1 s_2
   * 0:1.0:0.5 | a:1 b:1 c:1
   * | a:0.5 b:2 c:1
   */
  std::string example_string =
      // Header
      "_label,a,b,c,d,s_1,s_2\n"
      // Example 1
      "shared,1,0.5,,,,\n"
      "0:0.1:0.75,0.5,1,2,,,\n"
      ",1,,3,,,\n"
      ",,,,,,\n"
      // Example 2
      "shared,,,,,1,1\n"
      "0:1.0:0.5,1,1,1,,,\n"
      ",0.5,2,1,,,\n"
      ",,,,,,\n";

  auto vw = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--csv", "--cb_adf"));

  VW::io_buf buffer;
  buffer.add_file(VW::io::create_buffer_view(example_string.data(), example_string.size()));
  VW::multi_ex examples;

  // Example 1
  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_EQ(vw->example_parser->reader(vw.get(), buffer, examples), 1);
  EXPECT_EQ(examples[0]->l.cb.costs.size(), 1);
  EXPECT_EQ(examples[0]->feature_space[' '].size(), 2);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[0], 1);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[1], 0.5);
  VW::finish_example(*vw, *examples[0]);
  examples.clear();

  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_EQ(vw->example_parser->reader(vw.get(), buffer, examples), 1);
  EXPECT_EQ(examples[0]->l.cb.costs.size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->l.cb.costs[0].probability, 0.75);
  EXPECT_FLOAT_EQ(examples[0]->l.cb.costs[0].cost, 0.1);
  EXPECT_FLOAT_EQ(examples[0]->l.cb.costs[0].action, 0);
  EXPECT_EQ(examples[0]->feature_space[' '].size(), 3);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[0], 0.5);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[1], 1);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[2], 2);
  VW::finish_example(*vw, *examples[0]);
  examples.clear();

  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_EQ(vw->example_parser->reader(vw.get(), buffer, examples), 1);
  EXPECT_EQ(examples[0]->l.cb.costs.size(), 0);
  EXPECT_EQ(examples[0]->feature_space[' '].size(), 2);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[0], 1);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[1], 3);
  VW::finish_example(*vw, *examples[0]);
  examples.clear();

  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_EQ(vw->example_parser->reader(vw.get(), buffer, examples), 1);
  EXPECT_EQ(examples[0]->is_newline, true);
  VW::finish_example(*vw, *examples[0]);
  examples.clear();

  // Example 2
  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_EQ(vw->example_parser->reader(vw.get(), buffer, examples), 1);
  EXPECT_EQ(examples[0]->l.cb.costs.size(), 1);
  EXPECT_EQ(examples[0]->feature_space[' '].size(), 2);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[0], 1);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[1], 1);
  VW::finish_example(*vw, *examples[0]);
  examples.clear();

  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_EQ(vw->example_parser->reader(vw.get(), buffer, examples), 1);
  EXPECT_EQ(examples[0]->l.cb.costs.size(), 1);
  EXPECT_FLOAT_EQ(examples[0]->l.cb.costs[0].probability, 0.5);
  EXPECT_FLOAT_EQ(examples[0]->l.cb.costs[0].cost, 1.0);
  EXPECT_FLOAT_EQ(examples[0]->l.cb.costs[0].action, 0);
  EXPECT_EQ(examples[0]->feature_space[' '].size(), 3);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[0], 1);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[1], 1);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[2], 1);
  VW::finish_example(*vw, *examples[0]);
  examples.clear();

  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_EQ(vw->example_parser->reader(vw.get(), buffer, examples), 1);
  EXPECT_EQ(examples[0]->l.cb.costs.size(), 0);
  EXPECT_EQ(examples[0]->feature_space[' '].size(), 3);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[0], 0.5);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[1], 2);
  EXPECT_FLOAT_EQ(examples[0]->feature_space[' '].values[2], 1);
  VW::finish_example(*vw, *examples[0]);
  examples.clear();

  examples.push_back(&VW::get_unused_example(vw.get()));
  EXPECT_EQ(vw->example_parser->reader(vw.get(), buffer, examples), 1);
  EXPECT_EQ(examples[0]->is_newline, true);
  VW::finish_example(*vw, *examples[0]);
  examples.clear();
}
