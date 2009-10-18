#ifndef MS_H
#define MS_H

#include "example.h"
#include "parser.h"

struct prediction {
  float p;
  size_t example_number;
};

const size_t multindex = 5;

int receive_features(parser* p, void* ex);
void send_prediction(int sock, prediction pred);
bool blocking_get_prediction(int sock, prediction &p);

#endif
