#ifndef ECT_H
#define ECT_H

#include "oaa.h"
#include "parse_args.h"

namespace ECT
{
  void parse_flags(vw&, std::vector<std::string>&, po::variables_map&, po::variables_map& vm_file, size_t s);
}

#endif
