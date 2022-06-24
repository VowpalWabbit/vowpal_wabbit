// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw_clr.h"
#include "vw_base.h"

namespace VW
{
/// <summary>
/// VowpalWabbit model wrapper used in multi-threaded scenarios.
/// </summary>
public ref class VowpalWabbitModel : public VowpalWabbitBase
{
public:
  /// <summary>
  /// Initializes a new <see cref="VowpalWabbitModel"/> instance.
  /// </summary>
  /// <param name="settings">Arguments passed to native instance.</param>
  VowpalWabbitModel(VowpalWabbitSettings^ settings);

  /// <param name="args">Command line arguments passed to native instance.</param>
  VowpalWabbitModel(String^ args);
};
}
