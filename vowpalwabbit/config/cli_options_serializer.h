// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "config/options_serializer.h"
#include "config/option.h"

#include <string>
#include <sstream>

namespace VW
{
namespace config
{
struct cli_options_serializer : options_serializer_i, typed_option_visitor
{
  /**
   * @brief Construct a new cli options serializer object
   *
   * @param escape Whether to escape option values which have special
   * characters. ' " \ are escaped and strings with spaces are quoted. Not
   * escaping values can result in producing a command line that is ambiguous if
   * any of these chars are used.
   */
  explicit cli_options_serializer(bool escape);

  void add(base_option& option) override;
  std::string str() const override;
  size_t size() const override;

  void visit(typed_option<uint32_t>& option) override;
  void visit(typed_option<uint64_t>& option) override;
  void visit(typed_option<int32_t>& option) override;
  void visit(typed_option<int64_t>& option) override;
  void visit(typed_option<float>& option) override;
  void visit(typed_option<std::string>& option) override;
  void visit(typed_option<bool>& option) override;
  void visit(typed_option<std::vector<std::string>>& option) override;

private:
  std::stringstream m_output_stream;
  bool m_escape;
};

}  // namespace config
}  // namespace VW
