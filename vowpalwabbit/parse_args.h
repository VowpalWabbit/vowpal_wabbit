/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef PA_H
#define PA_H

#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>
namespace po = boost::program_options;
#include "gd.h"
#include "global_data.h"

vw parse_args(int argc, char *argv[]);

#endif
