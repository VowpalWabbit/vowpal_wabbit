/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef GD_MF_H
#define GD_MF_H

#include <math.h>
#include "example.h"
#include "parse_regressor.h"
#include "parser.h"
#include "gd.h"

namespace GDMF{
void drive_gd_mf(void*);
 void save_load(void* in, io_buf& model_file, bool read, bool text);
}
#endif
