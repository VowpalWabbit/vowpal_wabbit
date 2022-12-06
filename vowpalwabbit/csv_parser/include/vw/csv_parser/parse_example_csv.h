// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/text_utils.h"
#include "vw/config/option_group_definition.h"
#include "vw/core/global_data.h"
#include "vw/core/v_array.h"

#include <unordered_map>
#include <utility>
#include <vector>

namespace VW
{
namespace parsers
{
namespace csv
{

class csv_parser_options
{
public:
  bool enabled = false;
  // CSV parsing configurations
  std::string csv_separator = ",";
  bool csv_no_file_header = false;
  std::string csv_header = "";
  std::string csv_ns_value = "";
  bool csv_remove_outer_quotes = true;
};

int parse_csv_examples(VW::workspace* all, io_buf& buf, VW::multi_ex& examples);

class csv_parser : public VW::details::input_parser
{
public:
  std::vector<std::string> header_fn;
  std::vector<std::string> header_ns;
  size_t line_num = 0;
  csv_parser_options options;
  VW::v_array<size_t> label_list;
  VW::v_array<size_t> tag_list;
  std::unordered_map<std::string, VW::v_array<size_t>> feature_list;
  std::unordered_map<std::string, float> ns_value;

  explicit csv_parser(csv_parser_options options) : VW::details::input_parser("csv"), options(std::move(options)) {}
  ~csv_parser() override = default;

  static void set_parse_args(VW::config::option_group_definition& in_options, csv_parser_options& parsed_options);
  static void handle_parse_args(csv_parser_options& parsed_options);

  bool next(VW::workspace& all, io_buf& buf, VW::multi_ex& examples) override
  {
    return parse_csv(&all, examples[0], buf) != 0;
  }

private:
  static void set_csv_separator(std::string& str, const std::string& name);
  void reset();
  int parse_csv(VW::workspace* all, VW::example* ae, io_buf& buf);
  size_t read_line(VW::workspace* all, VW::example* ae, io_buf& buf);
};
}  // namespace csv
}  // namespace parsers
}  // namespace VW
