/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef VW_H
#define VW_H

#include "global_data.h"
#include "example.h"
#include "hash.h"

namespace VW {
  /*
    You must call initialize to get access to the library.  The argument is a vew commandline.  

    Caveats: 
    (1) Some commandline parameters do not make sense as a library.
    (2) The code is not yet reentrant.
   */
  vw initialize(string s);

  void cmd_string_replace_value( string& cmd, string flag_to_replace, string new_value );

  char** get_argv_from_string(string s, int& argc);

  /*
    Call finish() after you are done with the vw instance.  This cleans up memory usage.
   */
  void finish(vw& all);

  //The next commands deal with creating examples.  Caution: VW does not all allow creation of many examples at once by default.  You can adjust the exact number by tweaking ring_size.

  /* The simplest of two ways to create an example.  An example_line is the literal line in a VW-format datafile.
   */
  example* read_example(vw& all, char* example_line);

  //The more complex way to create an example.   
  typedef pair< unsigned char, vector<feature> > feature_space; //just a helper definition.
  struct primitive_feature_space { //just a helper definition.
    unsigned char name; 
    feature* fs; 
    size_t len; }; 

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

  //after you create and fill feature_spaces, get an example with everything filled in.
  example* import_example(vw& all, primitive_feature_space* features, size_t len);
  example* import_example(vw& all, vector< feature_space > ec_info);
  void parse_example_label(vw&all, example&ec, string label);
  example* new_unused_example(vw& all);
  void add_constant_feature(vw& all, example*ec);

  void add_label(example* ec, float label, float weight = 1, float base = 0);
  //notify VW that you are done with the example.
  void finish_example(vw& all, example* ec);
}

#endif
