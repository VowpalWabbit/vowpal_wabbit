// Copyright (c) by respective owners including Yahoo!)
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/print_utils.h"

#include "vw/io/errno_handling.h"
#include "vw/io/io_adapter.h"
#include "vw/io/logger.h"

namespace VW
{
namespace details
{
void global_print_newline(
    const std::vector<std::unique_ptr<VW::io::writer>>& final_prediction_sink, VW::io::logger& logger)
{
  const char temp = '\n';
  for (auto& sink : final_prediction_sink)
  {
    ssize_t t = sink->write(&temp, 1);
    if (t != 1) { logger.err_error("write error: {}", VW::io::strerror_to_string(errno)); }
  }
}
}  // namespace details
}  // namespace VW