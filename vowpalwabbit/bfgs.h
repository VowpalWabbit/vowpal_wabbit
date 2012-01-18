/*
Copyright (c) 2010 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef BFGS_H
#define BFGS_H
#include "gd.h"

namespace BFGS {

  void drive_bfgs();
  void initializer();
  void finish();
  void learn(example* ec);

}

#endif
