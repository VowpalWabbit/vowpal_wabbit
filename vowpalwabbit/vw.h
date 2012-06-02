#ifndef VW_H
#define VW_H

#include "global_data.h"
#include "example.h"

namespace VW {
  vw initialize(string s);
  void finish(vw& all);

  example* read_example(vw& all, char* example_line);

  typedef pair< unsigned char, vector<feature> > feature_space;
  example* import_example(vw& all, vector< feature_space > ec_info);

  void finish_example(vw& all, example* ec);
  inline uint32_t hasher(vw& all, string s, unsigned long u)
  {
    substring ss;
    ss.begin = (char*)s.c_str();
    ss.end = ss.begin + s.length();
    return all.p->hasher(ss,u);
  }
  const uint32_t hash_base = 97562527;
  
}

#endif
