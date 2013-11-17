/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef SCE
#define SCE

#include "io_buf.h"
#include "parse_primitives.h"
#include "example.h"
#include "vw.h"

parser* new_parser();
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include "global_data.h"
void parse_source_args(vw& all, po::variables_map& vm, bool quiet, size_t passes);

bool examples_to_finish();

//only call these from the library form:
void initialize_parser_datastructures(vw& all);
void release_parser_datastructures(vw& all);
void adjust_used_index(vw& all);

//parser control

void make_example_available();
bool parser_done(parser* p);

//source control functions
bool inconsistent_cache(size_t numbits, io_buf& cache);
void reset_source(vw& all, size_t numbits);
void finalize_source(parser* source);
void set_compressed(parser* par);
void initialize_examples(vw& all);
void free_parser(vw& all);

#endif
