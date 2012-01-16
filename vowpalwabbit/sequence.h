#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "oaa.h"

void parse_sequence_args(po::variables_map& vm, example* (**gf)(), void (**rf)(example*));

#endif
