/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */

#pragma once
#include "v_array.h"
#include "v_hashmap.h"
#include "parse_primitives.h"
#include "hash.h"

class labeldict {
  private:

  v_array<substring> id2name;
  v_hashmap<substring,uint32_t> name2id;
  uint32_t K;

  public:

  labeldict(string label_list) {
    id2name = v_init<substring>();
    substring ss = { (char*)label_list.c_str(), nullptr };
    ss.end = ss.begin + label_list.length();
    tokenize(',', ss, id2name);

    K = id2name.size();
    name2id.init(4 * K + 1, 0, substring_equal);
    for (size_t k=0; k<K; k++) {
      substring& l = id2name[k];
      size_t hash = uniform_hash((unsigned char*)l.begin, l.end-l.begin, 378401);
      uint32_t id = name2id.get(l, hash);
      if (id != 0) {
        cerr << "error: label dictionary initialized with multiple occurances of: ";
        for (char*c=l.begin; c!=l.end; c++) cerr << *c;
        cerr << endl;
        throw exception();
      }
      size_t len = l.end - l.begin;
      substring l_copy = { calloc_or_die<char>(len), nullptr };
      memcpy(l_copy.begin, l.begin, len * sizeof(char));
      l_copy.end = l_copy.begin + len;
      name2id.put(l_copy, hash, k+1);
    }
  }

  ~labeldict() {
    for (size_t i=0; i<id2name.size(); ++i)
      free(id2name[i].begin);
    id2name.delete_v();
  }

  uint32_t getK() { return K; }
  
  uint32_t get(substring& s) {
    size_t hash = uniform_hash((unsigned char*)s.begin, s.end-s.begin, 378401);
    uint32_t v  =  name2id.get(s, hash);
    if (v == 0) {
      cerr << "warning: missing named label '";
      for (char*c = s.begin; c != s.end; c++) cerr << *c;
      cerr << '\'' << endl;
    }
    return v;
  }

  substring get(uint32_t v) {
    if ((v == 0) || (v > K)) {
      substring ss = {nullptr,nullptr};
      return ss;
    } else
      return id2name[v-1];
  }
};

