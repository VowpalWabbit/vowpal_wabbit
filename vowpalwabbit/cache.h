/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef CACHE_H
#define CACHE_H

#include "parse_example.h"

const size_t int_size = 6;
const size_t char_size = 2;

char* run_len_decode(char *p, size_t& i);
char* run_len_encode(char *p, size_t i);

int read_cached_features(parser* p, void* ec);
void cache_tag(io_buf& cache, v_array<char> tag);
void cache_features(io_buf& cache, example* ae);
void output_byte(io_buf& cache, unsigned char s);
void output_features(io_buf& cache, unsigned char index, feature* begin, feature* end);

#endif
