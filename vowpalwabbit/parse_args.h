/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef PA_H
#define PA_H

#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>
namespace po = boost::program_options;
#include "gd.h"
#include "global_data.h"

vw* parse_args(int argc, char *argv[]);

namespace VW {
  /*
    You must call initialize to get access to the library.  The argument is a vew commandline.  

    Caveats: 
    (1) Some commandline parameters do not make sense as a library.
    (2) The code is not yet reentrant.
   */
  vw* initialize(string s);

  void cmd_string_replace_value( string& cmd, string flag_to_replace, string new_value );

  char** get_argv_from_string(string s, int& argc);

  /*
    Call finish() after you are done with the vw instance.  This cleans up memory usage.
   */
  void finish(vw& all);

}

#endif
