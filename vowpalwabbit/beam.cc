/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <iostream>
#include <float.h>
#include <stdio.h>
#include <math.h>
#include "beam.h"
#include "v_hashmap.h"
#include "v_array.h"

#define MULTIPLIER 5

using namespace std;

namespace Beam
{
  int compare_elem(const void *va, const void *vb) {
    // first sort on hash, then on loss
    elem* a = (elem*)va;
    elem* b = (elem*)vb;
    if (a->hash < b->hash) return -1; 
    if (a->hash > b->hash) return  1; 
    if (b->loss > a->loss) return 1; // if b is greater, it should go second
    else return -1;
  }

  beam::beam(bool (*eq)(state,state), size_t (*hs)(state), size_t max_beam_size) {
    equivalent = eq;
    hash = hs;
    empty_bucket = new v_array<elem>();
    last_retrieved = NULL;
    max_size = max_beam_size;
    losses = (float*)calloc(max_size, sizeof(float));
    dat = new v_hashmap<size_t,bucket>(8, empty_bucket, NULL);
  }

  beam::~beam() {
    // TODO: really free the elements
    delete dat;
    empty_bucket->delete_v();
    delete empty_bucket;
  }

  size_t hash_bucket(size_t id) { return 1043221*(893901 + id); }

  void beam::put(size_t id, state s, size_t hs, uint32_t act, float loss) {
    elem e = { s, hs, loss, id, last_retrieved, act, true };
    // check to see if we have this bucket yet
    bucket b = dat->get(id, hash_bucket(id));
    if (b->size() > 0) { // this one exists: just add to it
      b->push_back(e);
      //dat->put_after_get(id, hash_bucket(id), b);
      if (b->size() >= max_size * MULTIPLIER)
        prune(id);
    } else {
      bucket bnew = new v_array<elem>();
      bnew->push_back(e);
      dat->put_after_get(id, hash_bucket(id), bnew);
    }
  }

  void beam::put_final(state s, uint32_t act, float loss) {
    elem e = { s, 0, loss, 0, last_retrieved, act, true };
    final_states->push_back(e);
  }

  void beam::iterate(size_t id, void (*f)(beam*,size_t,state,float,void*), void*args) {
    bucket b = dat->get(id, hash_bucket(id));
    if (b->size() == 0) return;

    cout << "before prune" << endl;
    prune(id);
    cout << "after prune" << endl;

    for (elem*e=b->begin; e!=b->end; e++) {
      cout << "element" << endl;
      if (e->alive) {
        last_retrieved = e;
        f(this, id, e->s, e->loss, args);
      }
    }
  }

  #define SWAP(a,b) temp=(a);(a)=(b);(b)=temp;
  float quickselect(float *arr, size_t n, size_t k) {
    size_t i,ir,j,l,mid;
    float a,temp;

    l=0;
    ir=n-1;
    for(;;) {
      if (ir <= l+1) { 
        if (ir == l+1 && arr[ir] < arr[l]) {
          SWAP(arr[l],arr[ir]);
        }
        return arr[k];
      }
      else {
        mid=(l+ir) >> 1; 
        SWAP(arr[mid],arr[l+1]);
        if (arr[l] > arr[ir]) {
          SWAP(arr[l],arr[ir]);
        }
        if (arr[l+1] > arr[ir]) {
          SWAP(arr[l+1],arr[ir]);
        }
        if (arr[l] > arr[l+1]) {
          SWAP(arr[l],arr[l+1]);
        }
        i=l+1; 
        j=ir;
        a=arr[l+1]; 
        for (;;) { 
          do i++; while (arr[i] < a); 
          do j--; while (arr[j] > a); 
          if (j < i) break; 
          SWAP(arr[i],arr[j]);
        } 
        arr[l+1]=arr[j]; 
        arr[j]=a;
        if (j >= k) ir=j-1; 
        if (j <= k) l=i;
      }
    }
  }


  void beam::prune(size_t id) {
    bucket b = dat->get(id, hash_bucket(id));
    if (b->size() == 0) return;

    size_t num_alive = 0;
    if (equivalent == NULL) {
      for (size_t i=1; i<b->size(); i++) {
        (*b)[i].alive = true;
      }
      num_alive = b->size();
    } else {
      // first, sort on hash, backing off to loss
      qsort(b->begin, b->size(), sizeof(elem), compare_elem);

      // now, check actual equivalence
      size_t last_pos = 0;
      size_t last_hash = (*b)[0].hash;
      for (size_t i=1; i<b->size(); i++) {
        (*b)[i].alive = true;
        if ((*b)[i].hash != last_hash) {
          last_pos = i;
          last_hash = (*b)[i].hash;
        } else {
          for (size_t j=last_pos; j<i; j++) {
            if ((*b)[j].alive && equivalent((*b)[j].s, (*b)[i].s)) {
              (*b)[i].alive = false;
              break;
            }
          }
        }

        if ((*b)[i].alive) {
          losses[num_alive] = (*b)[i].loss;
          num_alive++;
        }
      }
    }

    if (num_alive <= max_size) return;

    // sort the remaining items on loss
    float cutoff = quickselect(losses, num_alive, max_size);
    bucket bnew = new v_array<elem>();
    for (elem*e=b->begin; e!=b->end; e++) {
      if (e->loss > cutoff) continue;
      bnew->push_back(*e);
      if (num_alive == 0) break;
      num_alive--;
    }
    dat->put_after_get(id, hash_bucket(id), bnew);
  }

  size_t beam::get_next_bucket(size_t start) {
    size_t next_bucket = 0;
    for (v_hashmap<size_t,bucket>::hash_elem* e=dat->dat.begin; e!=dat->dat.end_array; e++) {
      if (e->occupied) {
        size_t bucket_id = e->key;
        if ((bucket_id > start) && (bucket_id < next_bucket))
          next_bucket = bucket_id;
      }
    }
    return next_bucket;
  }

  void beam::get_best_output(std::vector<uint32_t>* action_seq) {
    action_seq->clear();
    if (final_states->size() == 0) {
      // TODO: error
      return;
    } else {
      elem *bestElem   = NULL;
      for (size_t i=0; i<final_states->size(); i++) {
        if ((bestElem == NULL) || ((*final_states)[i].loss < bestElem->loss))
          bestElem = &(*final_states)[i];
      }
      // chase backpointers
      while (bestElem != NULL) {
        std::vector<uint32_t>::iterator be = action_seq->begin();
        action_seq->insert( be, bestElem->last_action );
        bestElem = bestElem->backpointer;
      }
    }
  }


  struct test_beam_state {
    size_t id;
  };
  bool state_eq(state a,state b) { return ((test_beam_state*)a)->id == ((test_beam_state*)b)->id; }
  size_t state_hash(state a) { return 381049*(3820+((test_beam_state*)a)->id); }
  void expand_state(beam*b, size_t old_id, state old_state, float old_loss, void*args) {
    test_beam_state* new_state = (test_beam_state*)calloc(1, sizeof(test_beam_state));
    new_state->id = old_id + ((test_beam_state*)old_state)->id * 2;
    float new_loss = old_loss + 0.5f;
    cout << "expand_state " << old_loss << " -> " << new_state->id << " , " << new_loss << endl;
    b->put(old_id+1, new_state, 0, new_loss);
  }
  void test_beam() {
    beam*b = new beam(&state_eq, &state_hash, 5);
    for (size_t i=0; i<25; i++) {
      test_beam_state* s = (test_beam_state*)calloc(1, sizeof(test_beam_state));
      s->id = i / 3;
      b->put(0, s, 0, 0.f - (float)i);
      cout << "added " << s->id << endl;
    }
    b->iterate(0, expand_state, NULL);
  }
}
