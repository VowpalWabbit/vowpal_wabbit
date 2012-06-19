#ifndef BEAM_H
#define BEAM_H

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
    bool   alive;
  };

  typedef v_array<elem>* bucket;

  class beam {
  public:
    bool (*equivalent)(state, state);
    size_t (*hash)(state);

    v_hashmap<size_t, bucket>* dat;

    bucket empty_bucket;
    elem* last_retrieved;
    size_t max_size;
    float* losses;

    beam(bool (*eq)(state,state), size_t (*hs)(state), size_t max_beam_size);
    ~beam();
    void put(size_t id, state s, size_t hs, float loss);
    void put(size_t id, state s, float loss) { put(id, s, hash(s), loss); }
    void iterate(size_t id, void (*f)(beam*,size_t,state,float));
    void prune(size_t id);
  };
}

#endif
