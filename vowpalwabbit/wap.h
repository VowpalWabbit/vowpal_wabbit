#ifndef WAP_H
#define WAP_H

#include "io.h"
#include "parse_primitives.h"
#include "global_data.h"
#include "example.h"
#include "parse_args.h"

namespace WAP {
  void parse_flags(vw&, std::vector<std::string>&, po::variables_map& vm, po::variables_map& vm_file, size_t s);
}

#endif
