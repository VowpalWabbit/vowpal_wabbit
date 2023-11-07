// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/action_score.h"

#include "vw/common/string_view.h"
#include "vw/core/global_data.h"
#include "vw/core/io_buf.h"
#include "vw/core/model_utils.h"
#include "vw/core/text_utils.h"
#include "vw/core/v_array.h"
#include "vw/io/errno_handling.h"
#include "vw/io/logger.h"

void VW::details::print_action_score(
    VW::io::writer* f, const VW::v_array<action_score>& a_s, const VW::v_array<char>& tag, VW::io::logger& logger)
{
  if (f == nullptr) { return; }

  std::stringstream ss;
  ss << VW::to_string(a_s);
  if (!tag.empty()) { ss << " " << VW::string_view(tag.begin(), tag.size()); }
  ss << '\n';
  const auto ss_str = ss.str();
  ssize_t len = ss_str.size();
  ssize_t t = f->write(ss_str.c_str(), static_cast<unsigned int>(len));
  if (t != len) { logger.err_error("write error: {}", VW::io::strerror_to_string(errno)); }
}

std::ostream& VW::operator<<(std::ostream& os, const action_score& a_s)
{
  os << "(" << a_s.action << "," << a_s.score << ")";
  return os;
}

std::string VW::to_string(const action_scores& action_scores_or_probs, int decimal_precision)
{
  std::ostringstream ss;
  std::string delim;
  for (const auto& item : action_scores_or_probs)
  {
    ss << delim << fmt::format("{}:{}", item.action, VW::fmt_float(item.score, decimal_precision));
    delim = ",";
  }
  return ss.str();
}

size_t VW::model_utils::read_model_field(io_buf& io, action_score& a_s)
{
  size_t bytes = 0;
  bytes += read_model_field(io, a_s.action);
  bytes += read_model_field(io, a_s.score);
  return bytes;
}

size_t VW::model_utils::write_model_field(
    io_buf& io, const action_score a_s, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, a_s.action, upstream_name + "_action", text);
  bytes += write_model_field(io, a_s.score, upstream_name + "_score", text);
  return bytes;
}
