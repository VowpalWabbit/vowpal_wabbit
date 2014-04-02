/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef PARSE_ARGS_H
#define PARSE_ARGS_H

#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>
namespace po = boost::program_options;
#include "gd.h"
#include "global_data.h"

vw* parse_args(int argc, char *argv[]);

#endif
