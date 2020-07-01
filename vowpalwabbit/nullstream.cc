// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <fstream>

std::ostream* nullstream()
{
  static std::ofstream os;
  if (!os.is_open())
    os.open("/dev/null", std::ofstream::out | std::ofstream::app);
  return &os;
}
