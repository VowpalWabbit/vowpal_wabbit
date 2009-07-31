/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef SD_H
#define SD_H
#include <vector>

using namespace std;

struct static_data {
  size_t thread_bits; // log_2 of the number of threads.
  size_t num_bits; // log_2 of the number of features.
  size_t thread_mask; // 1 << num_bits >> thread_bits - 1.
  size_t mask; // 1 << num_bits -1
  vector<string> pairs; // pairs of features to cross.
  bool audit;
  
  size_t num_threads () { return 1 << thread_bits; };
  size_t length () { return 1 << num_bits; };

  char* program_name;
};
#endif
