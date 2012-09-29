/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
// This implements the allreduce function of MPI.  

#ifndef ALLREDUCE_H
#define ALLREDUCE_H
#include <string>

void all_reduce(float* buffer, int n, std::string master_location, size_t unique_id, size_t total, size_t node);

#endif
