/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef SCE
#define SCE

#include "io.h"
#include "parse_primitives.h"
#include "example.h"

const size_t wap_ldf_namespace  = 126;
const size_t history_namespace  = 127;
const size_t constant_namespace = 128;

parser* new_parser();
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include "global_data.h"
void parse_source_args(vw& all, po::variables_map& vm, bool quiet, size_t passes);

bool examples_to_finish();

//parser control

void start_parser(vw& all);
void end_parser(vw& all);
example* get_example(parser* pf);
void vw_finish_example(vw& all, example* ec);
void make_example_available();
bool parser_done(parser* p);

//source control functions
bool inconsistent_cache(size_t numbits, io_buf& cache);
void reset_source(vw& all, size_t numbits);
void finalize_source(parser* source);
void set_compressed(parser* par);

#endif
