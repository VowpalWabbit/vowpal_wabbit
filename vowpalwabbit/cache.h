/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include "v_array.h"
#include "io_buf.h"
#include "example.h"

const size_t int_size = 6;
const size_t char_size = 2;

char* run_len_decode(char *p, size_t& i);
char* run_len_encode(char *p, size_t i);

int read_cached_features(void*a, example* ec);
void cache_tag(io_buf& cache, v_array<char> tag);
void cache_features(io_buf& cache, example* ae, uint32_t mask);
void output_byte(io_buf& cache, unsigned char s);
void output_features(io_buf& cache, unsigned char index, feature* begin, feature* end, uint32_t mask);

