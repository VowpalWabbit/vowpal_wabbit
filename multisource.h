#ifndef MS_H
#define MS_H

#include "example.h"
#include "parser.h"

struct prediction {
  float p;
  int example_number;
};

const size_t multindex = 5;

int recieve_features(parser* p, void* ex);

#endif
