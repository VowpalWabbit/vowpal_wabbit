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

const size_t wap_ldf_namespace  = 126;
const size_t history_namespace  = 127;
const size_t constant_namespace = 128;
const size_t nn_output_namespace  = 129;
const size_t autolink_namespace  = 130;

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

namespace VW {
	void start_parser(vw& all, bool do_init = true);
	void end_parser(vw& all);
	example* get_example(parser* pf);
	
  // The simplest of two ways to create an example.  
  example* read_example(vw& all, char* example_line);

	//after you create and fill feature_spaces, get an example with everything filled in.
  example* import_example(vw& all, primitive_feature_space* features, size_t len);
  example* import_example(vw& all, vector< feature_space > ec_info);
  void parse_example_label(vw&all, example&ec, string label);
  example* new_unused_example(vw& all);
  void add_constant_feature(vw& all, example*ec);

  void add_label(example* ec, float label, float weight = 1, float base = 0);
  //notify VW that you are done with the example.
  void finish_example(vw& all, example* ec);

	primitive_feature_space* export_example(void* e, size_t& len);
	void releaseFeatureSpace(primitive_feature_space* features, size_t len);
}

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
