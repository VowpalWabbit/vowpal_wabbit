/*
Copyright (c) 2010 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef BFGS_H
#define BFGS_H
#include "gd.h"

namespace BFGS {

  void drive_bfgs(void*);
  void initializer(vw& all);
  void finish(vw& all);
  void learn(vw& all, example* ec);

}

#endif
