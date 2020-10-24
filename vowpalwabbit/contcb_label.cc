// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "contcb_label.h"
#include "vw.h"

namespace VW
{
namespace continuous_cb
{
void default_label(void* v)
{
  label& ld = static_cast<polylabel*>(v)->contcb;
  ld.action = FLT_MAX;
  ld.cost = FLT_MAX;
}

void parse_label(parser* p, shared_data*, void* v, std::vector<VW::string_view>& words)
{
  label& ld = static_cast<polylabel*>(v)->contcb;

  if (words.size() != 1) THROW("malformed label, expected: <action>:<cost>");

  tokenize(':', words[0], p->parse_name);
  if (p->parse_name.size() != 2) THROW("malformed label, expected: <action>:<cost>");

  ld.action = float_of_string(p->parse_name[0]);
  ld.cost = float_of_string(p->parse_name[1]);
}

void bufcache_label(label& ld, char* c)
{
  memcpy(c, &ld.action, sizeof(ld.action));
  c += sizeof(ld.action);
  memcpy(c, &ld.cost, sizeof(ld.cost));
}

void cache_label(void* v, io_buf& cache)
{
  label& ld = static_cast<polylabel*>(v)->contcb;
  char* c;
  cache.buf_write(c, sizeof(ld.action) + sizeof(ld.cost));
  bufcache_label(ld, c);
}

void bufread_label(label& ld, char* c)
{
  memcpy(&ld.action, c, sizeof(ld.action));
  c += sizeof(ld.action);
  memcpy(&ld.cost, c, sizeof(ld.cost));
}

size_t read_cached_label(shared_data*, void* v, io_buf& cache)
{
  label& ld = static_cast<polylabel*>(v)->contcb;
  char* c;
  size_t total = sizeof(ld.action) + sizeof(ld.cost);
  if (cache.buf_read(c, total) < total) return 0;
  bufread_label(ld, c);

  return total;
}

void delete_label(void*)
{ /* no-op */
}

float weight(void*) { return 1; }

bool test_label(void* v)
{
  label& ld = static_cast<polylabel*>(v)->contcb;
  return ld.action == FLT_MAX && ld.cost == FLT_MAX;
}

label_parser contcb_label = {default_label, parse_label, cache_label, read_cached_label, delete_label, weight, nullptr,
    test_label, sizeof(label)};

}  // namespace continuous_cb
}  // namespace VW