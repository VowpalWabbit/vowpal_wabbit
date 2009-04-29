/* 
Copyright (c) 2007 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef PR_H
#define PR_H
#include <vector.h>

using namespace std;

typedef float weight;

struct regressor {
  weight* weights;
  weight* other_weights;
  size_t numbits;
  size_t length;
  vector<string> pairs;
  bool seg;
};

void parse_regressor(vector<string> &regressors, regressor &r);

void dump_regressor(ofstream &o, regressor &r);

#endif
