#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "oaa.h"


#define clog_print_audit_features(ec) { print_audit_features(global.reg, ec); }

void parse_sequence_args(po::variables_map& vm, void (*base_l)(example*), void (*base_f)());
void drive_sequence();




/*
namespace BEAM {
  struct hash_item {
    uint32_t hash;
    size_t   cost_index;
  };

  struct cost_item {
    float    cost;
    size_t   hash_index;
    void*    elem;
  };

  struct beam {
    size_t mx_el;   // maximum number of elements
    size_t nm_el;   // total number of elements currently in beam
    uint32_t(*hs_el)(const void *);            // return a hash value for identity
    float(*cs_el)(const void *);               // return a cost
    int(*cmp_el)(const void *, const void *);  // compare two elements by identity

    // when update() is called, we need to first find the element
    // under consideration, according to identity.  this is the
    // find_item operation.  this will need to happen on every
    // update(), so needs to be very efficient.  if it's not found,
    // then it needs to be inserted; if it is found, it needs to
    // be updated.

    // we store things in two sorted arrays.  the first array is sorted
    // by cost and actually contains the elements.  this is then
    // used to return the to_array when requested.  the second array
    // contains mini-structures of hash value and a pointer into the
    // index into the first array, and is sorted on IDENTITY (not just
    // hash value).
    cost_item * cost_array;
    hash_item * hash_array;
  };

  beam* create(size_t mx_el, 
               uint32_t(*hs_el)(const void *),
               float(*cs_el)(const void *),
               int(*cmp_el)(const void *, const void *));
  void erase(beam* b);
  void free_all(beam* b);

  size_t num_elems(beam* b);
  float best_cost(beam* b);
  float worst_cost(beam* b);

  // if a cmp_el equivalent elem exists in the beam with a 
  // lower cost, update the cost; if it exists with a higher
  // cost, ignore it.  if it doesn't exist, insert it only if
  // it won't overfloat the beam.
  void update(beam* b, void* elem);

  // get the top mx_el out of the beam; return the number of elems
  // written to the array, which will be <= mx_el.
  size_t to_array(beam* b, void** array);
}
*/

#endif
