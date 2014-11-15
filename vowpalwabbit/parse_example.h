/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include <stdint.h>
#include "parse_primitives.h"
#include "example.h"

//example processing

int read_features(void* a, example* ex);// read example from  preset buffers.
void read_line(vw& all, example* ex, char* line);//read example from the line.
