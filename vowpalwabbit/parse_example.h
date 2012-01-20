/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef PE_H
#define PE_H
#include <stdint.h>
#include "io.h"
#include "parse_regressor.h"
#include "parse_primitives.h"
#include "parser.h"
#include "example.h"

//example processing
int read_features(parser* p, void* ex);

hash_func_t getHasher(const std::string& s);

#endif
