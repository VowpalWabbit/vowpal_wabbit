// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <cstdio>
#include <cfloat>
#include <cstdlib>
#include "v_array.h"

// TODO: special case the version where beam_size == 1
// TODO: *maybe* special case the version where beam_size <= 10

#define BEAM_CONSTANT_SIZE 0

namespace Beam
{
template <class T>
struct beam_element
{
  uint32_t hash;  // a cached hash value -- if a ~= b then h(a) must== h(b)
  float cost;     // cost of this element
  T *data;        // pointer to element data -- rarely accessed!
  bool active;    // is this currently active
  //  bool     recombined;                 // if we're not the BEST then we've been recombined
  //  v_array<T*> * recomb_friends;   // if we're the BEST (among ~= elements), then recomb_friends is everything that's
  //  equivalent to us but worse... NOT USED if we're not doing k-best predictions
};

inline int compare_on_cost(const void *void_a, const void *void_b)
{
  if (void_a == void_b)
    return 0;
  const beam_element<void> *a = (const beam_element<void> *)void_a;
  const beam_element<void> *b = (const beam_element<void> *)void_b;
  if (a->active && !b->active)
    return -1;  // active things come before inactive things
  else if (!a->active && b->active)
    return 1;
  else if (!a->active && !b->active)
    return 0;
  else if (a->cost < b->cost)
    return -1;  // otherwise sort by cost
  else if (a->cost > b->cost)
    return 1;
  else
    return 0;
}

inline int compare_on_hash_then_cost(const void *void_a, const void *void_b)
{
  if (void_a == void_b)
    return 0;
  const beam_element<void> *a = (const beam_element<void> *)void_a;
  const beam_element<void> *b = (const beam_element<void> *)void_b;
  if (a->active && !b->active)
    return -1;  // active things come before inactive things
  else if (!a->active && b->active)
    return 1;
  else if (!a->active && !b->active)
    return 0;
  else if (a->hash < b->hash)
    return -1;  // if the hashes are different, sort by hash
  else if (a->hash > b->hash)
    return 1;
  else if (a->cost < b->cost)
    return -1;  // otherwise sort by cost
  else if (a->cost > b->cost)
    return 1;
  else
    return 0;
}

template <class T>
class beam
{
 private:
  size_t beam_size;           // the beam size -- how many active elements can we have
  size_t count;               // how many elements do we have currently -- should be == to A.size()
  float pruning_coefficient;  // prune anything with cost >= pruning_coefficient * best, set to FLT_MAX to not do
                              // coefficient-based pruning
  float worst_cost;           // what is the cost of the worst (highest cost) item in the beam
  float best_cost;            // what is the cost of the best (lowest cost) item in the beam
  float prune_if_gt;          // prune any element with cost greater than this
  T *best_cost_data;          // easy access to best-cost item
  bool do_kbest;
  v_array<beam_element<T>> A;  // the actual data
  //  v_array<v_array<beam_element<T>*>> recomb_buckets;

  //  static size_t NUM_RECOMB_BUCKETS = 10231;

  bool (*is_equivalent)(T *, T *);  // test if two items are equivalent; nullptr means don't do hypothesis recombination

 public:
  beam(size_t beam_size, float prune_coeff = FLT_MAX, bool (*test_equiv)(T *, T *) = nullptr, bool kbest = false)
      : beam_size(beam_size), pruning_coefficient(prune_coeff), do_kbest(kbest), is_equivalent(test_equiv)
  {
    count = 0;
    worst_cost = -FLT_MAX;
    best_cost = FLT_MAX;
    prune_if_gt = FLT_MAX;
    best_cost_data = nullptr;
    A = v_init<beam_element<T>>();
    if (beam_size <= BEAM_CONSTANT_SIZE)
      A.resize(beam_size, true);
    else
      A.resize((beam_size + 1) * 4, true);
    if (beam_size == 1)
      do_kbest = false;  // automatically turn of kbest
  }

  inline bool might_insert(float cost) { return (cost <= prune_if_gt) && ((count < beam_size) || (cost < worst_cost)); }

  bool insert(T *data, float cost, uint32_t hash)  // returns TRUE iff element was actually added
  {
    if (!might_insert(cost))
      return false;

    // bool we_were_worse = false;
    // if (is_equivalent) {
    //   size_t mod = recomb_buckets.size();
    //   size_t id  = hash % mod;
    //   size_t equiv_pos = bucket_contains_equiv(recomb_buckets[i], data, hash);
    //   if (equiv_pos != (size_t) -1) { // we can recombing at equiv_pos
    //     if (cost >= recomb_buckets[i][equiv_pos].cost) {
    //       // we are more expensive, so ignore
    //       we_were_worse = true;
    //       beam_element<T> * be = new beam_element<T>;
    //       be->hash = hash; be->cost = cost; be->data = data; be->active = true; be->recombined = false;
    //       be->recomb_friends = nullptr; add_recomb_friend(recomb_buckets[i][equiv_pos], be);
    //   }
    // }

    if (beam_size < BEAM_CONSTANT_SIZE)
    {  // find the worst item and directly replace it
      size_t worst_idx = 0;
      float worst_idx_cost = A[0].cost;
      for (size_t i = 1; i < beam_size; i++)
        if (A[i].cost > worst_idx_cost)
        {
          worst_idx = i;
          worst_idx_cost = A[i].cost;
          if (worst_idx_cost <= worst_cost)
            break;
        }
      if (cost >= worst_idx_cost)
        return false;

      A[worst_idx].hash = hash;
      A[worst_idx].cost = cost;
      A[worst_idx].data = data;
      A[worst_idx].active = true;
      // A[worst_idx].recombined = false;
      // A[worst_idx].recomb_friends = nullptr;  // TODO: free it if it isn't nullptr
      worst_cost = cost;
    }
    else
    {
      beam_element<T> be;
      be.hash = hash;
      be.cost = cost;
      be.data = data;
      be.active = true;
      // be.recombined = false;
      // be.recomb_friends = nullptr;

      A.push_back(be);
      count++;
    }

    if (cost < best_cost)
    {
      best_cost = cost;
      best_cost_data = data;
    }
    if (cost > worst_cost)
    {
      worst_cost = cost;
      prune_if_gt = std::max(1.f, best_cost) * pruning_coefficient;
    }
    return true;
  }

  beam_element<T> *get_best_item()
  {
    if (count == 0)
      return nullptr;
    beam_element<T> *ret = A.begin;
    while ((ret != A.end) && (!ret->active)) ++ret;
    return (ret == A.end) ? nullptr : ret;
  }

  beam_element<T> *pop_best_item()
  {
    if (count == 0)
      return nullptr;

    beam_element<T> *ret = nullptr;
    float next_best_cost = FLT_MAX;
    for (beam_element<T> *el = A.begin; el != A.end; el++)
      if ((ret == nullptr) && el->active && (el->cost <= best_cost))
        ret = el;
      else if (el->active && (el->cost < next_best_cost))
      {
        next_best_cost = el->cost;
        best_cost_data = el->data;
      }

    if (ret != nullptr)
    {
      best_cost = next_best_cost;
      prune_if_gt = std::max(1.f, best_cost) * pruning_coefficient;
      ret->active = false;
      count--;
    }
    else
    {
      best_cost = FLT_MAX;
      prune_if_gt = FLT_MAX;
      best_cost_data = nullptr;
    }

    return ret;
  }

  void do_recombination()
  {
    qsort(A.begin, A.size(), sizeof(beam_element<T>), compare_on_hash_then_cost);
    size_t start = 0;
    while (start < A.size() - 1)
    {
      size_t end = start + 1;
      for (; (end < A.size()) && (A[start].hash == A[end].hash); end++)
        ;
      assert(start < A.size());
      assert(end <= A.size());
      // std::cerr << "start=" << start << " end=" << end << std::endl;
      // go over all pairs
      for (size_t i = start; i < end; i++)
      {
        if (!A[i].active)
          continue;
        assert(i < A.size());
        for (size_t j = i + 1; j < end; j++)
        {
          if (!A[j].active)
            continue;
          assert(j < A.size());
          // std::cerr << "te " << i << "," << j << std::endl;
          if (is_equivalent(A[i].data, A[j].data))
          {
            A[j].active = false;  // TODO: if kbest is on, do recomb_friends
            // std::cerr << "equivalent " << i << "," << j << ": " << ((size_t)A[i].data) << " and " <<
            // ((size_t)A[j].data)
            // << std::endl;
          }
        }
      }
      start = end;
    }
  }

  void compact(void (*free_data)(T *) = nullptr)
  {
    if (is_equivalent)
      do_recombination();
    qsort(A.begin, A.size(), sizeof(beam_element<T>), compare_on_cost);  // TODO: quick select

    if (count <= beam_size)
      return;

    count = beam_size;
    if (is_equivalent)  // we might be able to get rid of even more
      while ((count > 1) && !A[count - 1].active) count--;

    if (free_data)
      for (beam_element<T> *be = A.begin + count; be != A.end; ++be) free_data(be->data);

    A.end = A.begin + count;

    best_cost = A[0].cost;
    worst_cost = A[count - 1].cost;
    prune_if_gt = std::max(1.f, best_cost) * pruning_coefficient;
    best_cost_data = A[0].data;
  }

  void maybe_compact(void (*free_data)(T *) = nullptr)
  {
    if (count >= beam_size * 10)
      compact(free_data);
  }

  void erase(void (*free_data)(T *) = nullptr)
  {
    if (free_data)
      for (beam_element<T> *be = A.begin; be != A.end; ++be) free_data(be->data);
    A.erase();
    count = 0;
    worst_cost = -FLT_MAX;
    best_cost = FLT_MAX;
    prune_if_gt = FLT_MAX;
    best_cost_data = nullptr;
  }

  ~beam()
  {
    assert(A.size() == 0);
    A.delete_v();
  }

  beam_element<T> *begin() { return A.begin; }
  beam_element<T> *end() { return A.end; }
  size_t size() { return count; }
  bool empty() { return A.empty(); }
  size_t get_beam_size() { return beam_size; }

 private:
  // void add_recomb_friend(beam_element<T> *better, beam_element<T> *worse) {
  //   assert( better->cost <= worse->cost );
  //   if (better->recomb_friends == nullptr) {
  //     if (worse->recomb_friends != nullptr) {
  //       better->recomb_friends = worse->recomb_friends;
  //       worse->recomb_friends = nullptr;
  //     } else
  //       better->recomb_friends = new std::vector<beam_element<T>*>;
  //   } else {
  //     assert(worse->recomb_friends == nullptr);
  //   }
  // }
};

}  // namespace Beam
