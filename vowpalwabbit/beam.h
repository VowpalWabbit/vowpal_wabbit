/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef BEAM_H
#define BEAM_H

#include <stdio.h>
#include <float.h>
#include <stdlib.h>
#include "v_array.h"

using namespace std;

namespace Beam {

struct beam_element {
  uint32_t hash;   // a cached hash value -- if a ~= b then h(a) must== h(b)
  float    cost;   // cost of this element
  void*    data;   // pointer to element data -- rarely accessed!
  bool     active; // is this currently active
  /* TODO: recombination
  bool     recombined;                 // if we're not the BEST then we've been recombined
  v_array<void*> * recomb_friends;   // if we're the BEST (among ~= elements), then recomb_friends is everything that's equivalent to us but worse... NOT USED if we're not doing k-best predictions
  */
};

inline int compare_on_cost(const void *void_a, const void *void_b) {
  const beam_element *a = (const beam_element*) void_a;
  const beam_element *b = (const beam_element*) void_b;
  if      ( a->active && !b->active) return -1;   // active things come before inactive things
  else if (!a->active &&  b->active) return  1;
  else if (!a->active && !b->active) return  0;
  else if (a->cost < b->cost) return -1;          // otherwise sort by cost
  else if (a->cost > b->cost) return  1;
  else return 0;
}

inline int compare_on_hash_then_cost(const void *void_a, const void *void_b) {
  const beam_element *a = (const beam_element*) void_a;
  const beam_element *b = (const beam_element*) void_b;
  if      ( a->active && !b->active) return -1;   // active things come before inactive things
  else if (!a->active &&  b->active) return  1;
  else if (!a->active && !b->active) return  0;
  else if (a->hash < b->hash) return -1;          // if the hashes are different, sort by hash
  else if (a->hash > b->hash) return  1;
  else if (a->cost < b->cost) return -1;          // otherwise sort by cost
  else if (a->cost > b->cost) return  1;
  else return 0;
}

class beam {
 private:
  size_t beam_size;   // the beam size -- how many active elements can we have
  size_t count;       // how many elements do we have currently -- should be == to A.size()
  float  pruning_coefficient;  // prune anything with cost >= pruning_coefficient * best, set to FLT_MAX to not do coefficient-based pruning
  float  worst_cost;  // what is the cost of the worst (highest cost) item in the beam
  float  best_cost;   // what is the cost of the best (lowest cost) item in the beam
  float  prune_if_gt; // prune any element with cost greater than this
  void*  best_cost_data;  // easy access to best-cost item
  v_array<beam_element> A; // the actual data
  
  bool (*is_equivalent)(void*,void*);  // test if two items are equivalent; NULL means don't do hypothesis recombination
  
 public:
  beam(size_t beam_size, float prune_coeff=FLT_MAX, bool (*test_equiv)(void*,void*)=NULL)
      : beam_size(beam_size)
      , pruning_coefficient(prune_coeff)
      , is_equivalent(test_equiv) {
    count = 0;
    worst_cost  = -FLT_MAX;
    best_cost   =  FLT_MAX;
    prune_if_gt =  FLT_MAX;
    best_cost_data = NULL;
    A.resize((beam_size+1) * 4, true);
  }

  bool insert(void*data, float cost, uint32_t hash) { // returns TRUE iff element was actually added
    bool should_add = false;

    if (count < beam_size) should_add = true;
    else if (cost < worst_cost) should_add = true;
    if (cost > prune_if_gt) should_add = false;
    
    //cerr << "insert " << ((size_t)data) << " with cost=" << cost << " wc=" << worst_cost << " count=" << count << " size=" << beam_size << " has should_add=" << should_add << endl;
    
    if (!should_add) return false;

    beam_element be;
    be.hash = hash;
    be.cost = cost;
    be.data = data;
    be.active = true;

    A.push_back(be);
    count++;
    
    if (cost < best_cost) {
      best_cost = cost;
      best_cost_data = data;
    }
    if (cost > worst_cost) {
      worst_cost  = cost;
      prune_if_gt = max(1., best_cost) * pruning_coefficient;
    }

    return true;
  }

  void do_recombination() {
    qsort(A.begin, A.size(), sizeof(beam_element), compare_on_hash_then_cost);
    size_t start = 0;
    while (start < A.size() - 1) {
      size_t end = start+1;
      for (; (end < A.size()) && (A[start].hash == A[end].hash); end++);
      assert(start < A.size());
      assert(end <= A.size());
      //cerr << "start=" << start << " end=" << end << endl;
      // go over all pairs
      for (size_t i=start; i<end; i++) {
        if (! A[i].active) continue;
        assert(i < A.size());
        for (size_t j=i+1; j<end; j++) {
          if (! A[j].active) continue;
          assert(j < A.size());
          //cerr << "te " << i << "," << j << endl;
          if (is_equivalent(A[i].data, A[j].data)) {
            A[j].active = false; // TODO: if kbest is on, do recomb_friends
            //cerr << "equivalent " << i << "," << j << ": " << ((size_t)A[i].data) << " and " << ((size_t)A[j].data) << endl;
          }
        }
      }
      start = end;
    }
  }
  
  void compact(void (*free_data)(void*)=NULL) {
    if (count <= beam_size) return;

    if (is_equivalent) do_recombination();

    qsort(A.begin, A.size(), sizeof(beam_element), compare_on_cost); // TODO: quick select

    count = beam_size;
    if (is_equivalent) // we might be able to get rid of even more
      while ((count > 1) && !A[count-1].active) count--;
    
    if (free_data)
      for (beam_element * be = A.begin+count; be != A.end; ++be)
        free_data(be->data);

    A.end = A.begin + count;

    best_cost = A[0].cost;
    worst_cost = A[count-1].cost;
    prune_if_gt = max(1., best_cost) * pruning_coefficient;
    best_cost_data = A[0].data;
  }

  void maybe_compact(void (*free_data)(void*)=NULL) {
    if (count >= beam_size * 10)
      compact(free_data);
  }

  void erase(void (*free_data)(void*)=NULL) {
    if (free_data)
      for (beam_element * be = A.begin; be != A.end; ++be)
        free_data(be->data);
    A.erase();
  }

  ~beam() {
    assert(A.size() == 0);
    A.delete_v();
  }
  
  beam_element * begin() { return A.begin; }
  beam_element * end()   { return A.end; }
  size_t         size()  { return count; }
};



}


#endif









