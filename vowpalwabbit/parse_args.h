/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef PA_H
#define PA_H

#include <boost/program_options.hpp>
namespace po = boost::program_options;
#include "gd.h"

po::variables_map parse_args(int argc, char *argv[], boost::program_options::options_description& desc,
			     gd_vars& vars,
			     regressor &r, parser* p,
			     std::string &final_regressor_name);

#endif
