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

	// An example_line is the literal line in a VW-format datafile.
  // The more complex way to create an example.   
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

  inline float get_weight(vw& all, uint32_t index) 
  { return all.reg.weight_vector[(index * all.reg.stride) & all.reg.weight_mask];}

  inline uint32_t num_weights(vw& all) 
  { return (uint32_t)all.length();}
}


#endif
