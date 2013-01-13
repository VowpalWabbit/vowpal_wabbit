/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef BEAM_H
#define BEAM_H

#include <vector>
#include <stdio.h>
#include "v_hashmap.h"
#include "v_array.h"

typedef void* state;

namespace Beam
{
  struct elem {
    state  s;
    size_t hash;
    float  loss;
    size_t bucket_id;
    elem*  backpointer;
    uint32_t last_action;
    bool   alive;
  };

  typedef v_array<elem>* bucket;

  class beam {
  private:
    bool (*equivalent)(state, state);
    size_t (*hash)(state);

    v_hashmap<size_t, bucket>* dat;
    bucket final_states;

    bucket empty_bucket;
    elem* last_retrieved;
    size_t max_size;
    float* losses;

  public:
    beam(bool (*eq)(state,state), size_t (*hs)(state), size_t max_beam_size);
    ~beam();
    void put(size_t id, state s, size_t hs, uint32_t act, float loss);
    void put(size_t id, state s, uint32_t act, float loss) { put(id, s, hash(s), act, loss); }
    void put_final(state s, uint32_t act, float loss);
    void iterate(size_t id, void (*f)(beam*,size_t,state,float,void*), void*);
    void prune(size_t id);
    size_t get_next_bucket(size_t start);
    void get_best_output(std::vector<uint32_t>*);
  };
}

#endif
