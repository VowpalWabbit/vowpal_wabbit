/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
*/

#ifndef PP
#define PP

#include<iostream>
#include <stdint.h>
#include <math.h>
#include "v_array.h"
#include "io.h"
#include "example.h"

struct substring {
  char *begin;
  char *end;
};

struct shared_data {
  size_t queries;

  uint64_t example_number;
  uint64_t total_features;

  double t;
  double weighted_examples;
  double weighted_unlabeled_examples;
  double old_weighted_examples;
  double weighted_labels;
  double sum_loss;
  double sum_loss_since_last_dump;
  float dump_interval;// when should I update for the user.
  double gravity;
  double contraction;
  double min_label;//minimum label encountered
  double max_label;//maximum label encountered

  bool binary_label;
  uint32_t k;
};

struct label_parser {
  void (*default_label)(void*);
  void (*parse_label)(shared_data*, void*, v_array<substring>&);
  void (*cache_label)(void*, io_buf& cache);
  size_t (*read_cached_label)(shared_data*, void*, io_buf& cache);
  void (*delete_label)(void*);
  float (*get_weight)(void*);
  float (*get_initial)(void*);
  size_t label_size;
};

typedef size_t (*hash_func_t)(substring, unsigned long);

struct parser {
  v_array<substring> channels;//helper(s) for text parsing
  v_array<substring> words;
  v_array<substring> name;

  io_buf* input; //Input source(s)
  int (*reader)(void*, example* ae);
  hash_func_t hasher;
  bool resettable; //Whether or not the input can be reset.
  io_buf* output; //Where to output the cache.
  bool write_cache; 
  bool sort_features;
  bool sorted_cache;

  size_t ring_size;
  uint64_t parsed_examples; // The index of the parsed example.
  uint64_t local_example_number; 

  v_array<size_t> ids; //unique ids for sources
  v_array<size_t> counts; //partial examples received from sources
  size_t finished_count;//the number of finished examples;
  int label_sock;
  int bound_sock;
  int max_fd;

  label_parser* lp;  // moved from vw
};

//chop up the string into a v_array of substring.
void tokenize(char delim, substring s, v_array<substring> &ret);

inline char* safe_index(char *start, char v, char *max)
{
  while (start != max && *start != v)
    start++;
  return start;
}

inline void print_substring(substring s)
{
  std::cout.write(s.begin,s.end - s.begin);
}

// The following function is a home made strtof. The
// differences are :
//  - much faster (around 50% but depends on the string to parse)
//  - less error control, but utilised inside a very strict parser
//    in charge of error detection.
inline float parseFloat(char * p, char **end)
{
  char* start = p;

  if (!*p)
    return 0;
  int s = 1;
  while (*p == ' ') p++;
  
  if (*p == '-') {
    s = -1; p++;
  }
  
  float acc = 0;
  while (*p >= '0' && *p <= '9')
    acc = acc * 10 + *p++ - '0';
  
  int num_dec = 0;
  if (*p == '.') {
    p++;
    while (*p >= '0' && *p <= '9') {
      acc = acc *10 + (*p++ - '0') ;
      num_dec++;
    }
  }
  int exp_acc = 0;
  if(*p == 'e' || *p == 'E'){
    p++;
    int exp_s = 1;
    if (*p == '-') {
      exp_s = -1; p++;
    }
    while (*p >= '0' && *p <= '9')
      exp_acc = exp_acc * 10 + *p++ - '0';
    exp_acc *= exp_s;
    
  }
  if (*p == ' ')//easy case succeeded.
    {
      acc *= powf(10,(float)(exp_acc-num_dec));
      *end = p;
      return s * acc;
    }
  else
    return (float)strtod(start,end);
}

inline bool nanpattern( float value ) { return ((*(uint32_t*)&value) & 0x7fffffff) > 0x7f800000; } 

inline float float_of_substring(substring s)
{
  char* endptr = s.end;
  float f = parseFloat(s.begin,&endptr);
  if ((endptr == s.begin && s.begin != s.end) || nanpattern(f))
    {
      std::cout << "warning: " << std::string(s.begin, s.end-s.begin).c_str() << " is not a good float, replacing with 0" << std::endl;
      f = 0;
    }
  return f;
}

inline int int_of_substring(substring s)
{
  return atoi(std::string(s.begin, s.end-s.begin).c_str());
}

#endif
