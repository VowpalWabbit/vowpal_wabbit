/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef PARSE_EXAMPLE_H
#define PARSE_EXAMPLE_H
#include <stdint.h>
#include "parse_regressor.h"
#include "parse_primitives.h"
#include "parser.h"
#include "example.h"

//example processing

int read_features(void* a, example* ex);// read example from  preset buffers.
void read_line(vw& all, example* ex, char* line);//read example from the line.
size_t hashstring (substring s, uint32_t h);

hash_func_t getHasher(const std::string& s);

#endif
