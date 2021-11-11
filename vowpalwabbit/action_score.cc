// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "action_score.h"

#include "v_array.h"
#include "io_buf.h"
#include "global_data.h"
#include "model_utils.h"

#include "io/logger.h"

namespace logger = VW::io::logger;

namespace ACTION_SCORE
{
void print_action_score(VW::io::writer* f, const v_array<action_score>& a_s, const v_array<char>& tag)
{
  if (f == nullptr) { return; }

  std::stringstream ss;

  for (size_t i = 0; i < a_s.size(); i++)
  {
    if (i > 0) ss << ',';
    ss << a_s[i].action << ':' << a_s[i].score;
  }
  print_tag_by_ref(ss, tag);
  ss << '\n';
  const auto ss_str = ss.str();
  ssize_t len = ss_str.size();
  ssize_t t = f->write(ss_str.c_str(), static_cast<unsigned int>(len));
  if (t != len) logger::errlog_error("write error: {}", VW::strerror_to_string(errno));
}

std::ostream& operator<<(std::ostream& os, const action_score& a_s)
{
  os << "(" << a_s.action << "," << a_s.score << ")";
  return os;
}
}  // namespace ACTION_SCORE

namespace VW
{
namespace model_utils
{
size_t read_model_field(io_buf& io, ACTION_SCORE::action_score& as)
{
  size_t bytes = 0;
  bytes += read_model_field(io, as.action);
  bytes += read_model_field(io, as.score);
  return bytes;
}
size_t write_model_field(io_buf& io, const ACTION_SCORE::action_score& as, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, as.action, upstream_name + "_action", text);
  bytes += write_model_field(io, as.score, upstream_name + "_score", text);
  return bytes;
}
}  // namespace model_utils
}  // namespace VW
