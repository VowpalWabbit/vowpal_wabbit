// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "option.h"
#include "options_serializer.h"

#include <sstream>
#include <string>

namespace VW
{
namespace config
{
class cli_options_serializer : public options_serializer_i, typed_option_visitor
{
public:
  cli_options_serializer();

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
};

}  // namespace config
}  // namespace VW
