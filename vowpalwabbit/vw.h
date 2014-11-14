/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include "global_data.h"
#include "example.h"
#include "hash.h"
#include "simple_label.h"

namespace VW {

/*    Caveats:
    (1) Some commandline parameters do not make sense as a library.
    (2) The code is not yet reentrant.
   */
  vw* initialize(string s);

  void cmd_string_replace_value( string& cmd, string flag_to_replace, string new_value );

  char** get_argv_from_string(string s, int& argc);

  /*
    Call finish() after you are done with the vw instance.  This cleans up memory usage.
   */
  void finish(vw& all, bool delete_all=true);

  void start_parser(vw& all, bool do_init = true);
  void end_parser(vw& all);

  typedef pair< unsigned char, vector<feature> > feature_space; //just a helper definition.
  struct primitive_feature_space { //just a helper definition.
    unsigned char name;
    feature* fs;
    size_t len;
  };

  //The next commands deal with creating examples.  Caution: VW does not all allow creation of many examples at once by default.  You can adjust the exact number by tweaking ring_size.

  /* The simplest of two ways to create an example.  An example_line is the literal line in a VW-format datafile.
   */
  example<void>* read_example(vw& all, char* example_line);
  example<void>* read_example(vw& all, string example_line);

  //The more complex way to create an example.

  //after you create and fill feature_spaces, get an example with everything filled in.
  example<void>* import_example(vw& all, primitive_feature_space* features, size_t len);
  example<void>* import_example(vw& all, vector< feature_space > ec_info);
  void parse_example_label(vw&all, example<void>&ec, string label);
  void setup_example(vw& all, example<void>* ae);
  example<void>* new_unused_example(vw& all);
  example<void>* get_example(parser* pf);
  float get_topic_prediction(example<void>* ec, size_t i);//i=0 to max topic -1
  float get_label(example<void>* ec);
  float get_importance(example<void>* ec);
  float get_initial(example<void>* ec);
  float get_prediction(example<void>* ec);
  size_t get_tag_length(example<void>* ec);
  const char* get_tag(example<void>* ec);
  size_t get_feature_number(example<void>* ec);
  feature* get_features(vw& all, example<void>* ec, size_t& feature_number);
  void return_features(feature* f);

  void add_constant_feature(vw& all, example<void>* ec);
  void add_label(example<void>* ec, float label, float weight = 1, float base = 0);

  //notify VW that you are done with the example.
  void finish_example(vw& all, example<void>* ec);

  void copy_example_data(bool audit, example<void>*, example<void>*, size_t, void(*copy_label)(void*&,void*));
  void copy_example_data(bool audit, example<void>*, example<void>*);  // don't copy the label

  // after export_example, must call releaseFeatureSpace to free native memory
  primitive_feature_space* export_example(vw& all, example<void>* e, size_t& len);
  void releaseFeatureSpace(primitive_feature_space* features, size_t len);

  // inlines

  //First create the hash of a namespace.
  inline uint32_t hash_space(vw& all, string s)
  {
    substring ss;
    ss.begin = (char*)s.c_str();
    ss.end = ss.begin + s.length();
    return (uint32_t)all.p->hasher(ss,hash_base);
  }
  //Then use it as the seed for hashing features.
  inline uint32_t hash_feature(vw& all, string s, unsigned long u)
  {
    substring ss;
    ss.begin = (char*)s.c_str();
    ss.end = ss.begin + s.length();
    return (uint32_t)(all.p->hasher(ss,u) & all.parse_mask);
  }

  inline uint32_t hash_feature_cstr(vw& all, char* fstr, unsigned long u)
  {
    substring ss;
    ss.begin = fstr;
    ss.end = ss.begin + strlen(fstr);
    return (uint32_t)(all.p->hasher(ss,u) & all.parse_mask);
  }

  inline float get_weight(vw& all, uint32_t index, uint32_t offset)
  { return all.reg.weight_vector[(((index << all.reg.stride_shift) + offset) & all.reg.weight_mask)];}

  inline void set_weight(vw& all, uint32_t index, uint32_t offset, float value)
  { all.reg.weight_vector[(((index << all.reg.stride_shift) + offset) & all.reg.weight_mask)] = value;}

  inline uint32_t num_weights(vw& all)
  { return (uint32_t)all.length();}

  inline uint32_t get_stride(vw& all)
  { return (uint32_t)(1 << all.reg.stride_shift);}

  inline void update_dump_interval(vw& all) {
      if (all.progress_add) { 
        all.sd->dump_interval = (float)all.sd->weighted_examples + all.progress_arg;
      } else {
        all.sd->dump_interval = (float)all.sd->weighted_examples * all.progress_arg;
      }
  }
}
