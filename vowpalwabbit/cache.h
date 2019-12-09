// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "v_array.h"
#include "io_buf.h"
#include "example.h"

char* run_len_decode(char* p, size_t& i);
char* run_len_encode(char* p, size_t i);

int read_cached_features(vw* all, v_array<example*>& examples);
void cache_tag(io_buf& cache, v_array<char> tag);
void cache_features(io_buf& cache, example* ae, uint64_t mask);
void output_byte(io_buf& cache, unsigned char s);
void output_features(io_buf& cache, unsigned char index, features& fs, uint64_t mask);

namespace VW
{
uint32_t convert(size_t number);
}
