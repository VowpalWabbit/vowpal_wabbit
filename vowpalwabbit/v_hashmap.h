#ifndef V_HASHMAP_H
#define V_HASHMAP_H

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "v_array.h"

template<class K, class V> class v_hashmap{
 public:
  struct elem {
    bool   occupied;
    K      key;
    V      val;
    size_t hash;
  };

  bool (*equivalent)(K,K);
  size_t (*hash)(K);
  V default_value;
  v_array<elem> dat;
  size_t last_position;
  size_t num_occupants;


  size_t base_size() {
    return dat.end_array-dat.begin;
  }

  v_hashmap(size_t min_size, V def, bool (*eq)(K,K)) {
    dat = v_array<elem>();
    if (min_size < 1023) min_size = 1023;
    reserve(dat, min_size); // reserve sets to 0 ==> occupied=false

    default_value = def;
    equivalent = eq;

    last_position = 0;
    num_occupants = 0;
  }

  void set_equivalent(bool (*eq)(K,K)) { equivalent = eq; }

  ~v_hashmap() {
    //std::cerr << "~v_hashmap" << std::endl;
    dat.erase();
    free(dat.begin);
  }

  void clear() {
    memset(dat.begin, 0, base_size()*sizeof(elem));
    last_position = 0;
    num_occupants = 0;
  }

  void iter(void (*func)(K,V)) {
    //for (size_t lp=0; lp<base_size(); lp++) {
    for (elem* e=dat.begin; e!=dat.end_array; e++) {
      //elem* e = dat.begin+lp;
      if (e->occupied) {
        //printf("  [lp=%d\tocc=%d\thash=%zu]\n", lp, e->occupied, e->hash);
        func(e->key, e->val);
      }
    }
  }
    

  void put_after_get_nogrow(K key, size_t hash, V val) {
    //printf("++[lp=%d\tocc=%d\thash=%zu]\n", last_position, dat[last_position].occupied, hash);
    dat[last_position].occupied = true;
    dat[last_position].key = key;
    dat[last_position].val = val;
    dat[last_position].hash = hash;
  }

  void double_size() {
    //    printf("doubling size!\n");
    // remember the old occupants
    v_array<elem>tmp = v_array<elem>();
    reserve(tmp, num_occupants+10);
    for (elem* e=dat.begin; e!=dat.end_array; e++)
      if (e->occupied)
        push(tmp, *e);
    
    // double the size and clear
    reserve(dat, base_size()*2);
    memset(dat.begin, 0, base_size()*sizeof(elem));

    // re-insert occupants
    for (elem* e=tmp.begin; e!=tmp.end; e++) {
      get(e->key, e->hash);
      //      std::cerr << "reinserting " << e->key << " at " << last_position << std::endl;
      put_after_get_nogrow(e->key, e->hash, e->val);
    }
    tmp.erase();
    free(tmp.begin);
  }

  V get(K key, size_t hash) {
    size_t sz  = base_size();
    size_t first_position = hash % sz;
    last_position = first_position;
    while (true) {
      // if there's nothing there, obviously we don't contain it
      if (!dat[last_position].occupied)
        return default_value;

      // there's something there: maybe it's us
      if ((dat[last_position].hash == hash) &&
          ((equivalent == NULL) ||
           (equivalent(key, dat[last_position].key))))
        return dat[last_position].val;

      // there's something there that's NOT us -- advance pointer
      last_position++;
      if (last_position >= sz)
        last_position = 0;

      // check to make sure we haven't cycled around -- this is a bug!
      if (last_position == first_position) {
        std::cerr << "error: v_hashmap did not grow enough!" << std::endl;
        exit(-1);
      }
    }
  }

  // only call put_after_get(key, hash, val) if you've already
  // run get(key, hash).  if you haven't already run get, then
  // you should use put() rather than put_after_get().  these
  // both will overwrite previous values, if they exist.
  void put_after_get(K key, size_t hash, V val) {
    if (!dat[last_position].occupied) {
      num_occupants++;
      if (num_occupants*4 >= base_size()) {        // grow when we're a quarter full
        double_size();
        get(key, hash);  // probably should change last_position-- this is the lazy man's way to do it
      }
    }

    // now actually insert it
    put_after_get_nogrow(key, hash, val);
  }

  void put(K key, size_t hash, V val) {
    get(key, hash);
    put_after_get(key, hash, val);
  }
};

void test_v_hashmap();


#endif
