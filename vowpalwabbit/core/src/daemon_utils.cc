// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/daemon_utils.h"

#include "vw/io/errno_handling.h"
#include "vw/io/io_adapter.h"

namespace
{
class global_prediction
{
public:
  float p;
  float weight;
};

void send_prediction(VW::io::writer* f, global_prediction p)
{
  if (f->write(reinterpret_cast<const char*>(&p), sizeof(p)) < static_cast<int>(sizeof(p)))
    THROWERRNO("send_prediction write(unknown socket fd)");
}

size_t really_read(VW::io::reader* sock, void* in, size_t count)
{
  char* buf = static_cast<char*>(in);
  size_t done = 0;
  ssize_t r = 0;
  while (done < count)
  {
    if ((r = sock->read(buf, static_cast<unsigned int>(count - done))) == 0) { return 0; }

    if (r < 0) { THROWERRNO("read(" << sock << "," << count << "-" << done << ")"); }
    else
    {
      done += r;
      buf += r;
    }
  }
  return done;
}
}  // namespace

void VW::details::binary_print_result_by_ref(
    VW::io::writer* f, float res, float weight, const VW::v_array<char>& /* tag */, VW::io::logger&)
{
  if (f != nullptr)
  {
    global_prediction ps = {res, weight};
    send_prediction(f, ps);
  }
}

void VW::details::get_prediction(VW::io::reader* f, float& res, float& weight)
{
  global_prediction p{};
  really_read(f, &p, sizeof(p));
  res = p.p;
  weight = p.weight;
}