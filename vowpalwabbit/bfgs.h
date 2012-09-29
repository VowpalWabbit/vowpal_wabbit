/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef BFGS_H
#define BFGS_H
#include "gd.h"

namespace BFGS {

  void drive_bfgs(void*);
  void initializer(vw& all);
  void finish(void*);
  void learn(void*, example* ec);

}

#endif
