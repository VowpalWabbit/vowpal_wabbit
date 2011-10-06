/*
Copyright (c) 2011 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license

This implements various accumulate functions building on top of allreduce.  

*/

#ifndef ACCUMULATE_H
#define ACCUMULATE_H

#include "allreduce.h"
#include "global_data.h"

void accumulate(std::string master_location, regressor& reg, size_t o);
float accumulate_scalar(std::string master_location, float local_sum);
void accumulate_weighted_avg(std::string master_location, regressor& reg);
void accumulate_avg(std::string master_location, regressor& reg, size_t o);
double get_comm_time();

#endif
